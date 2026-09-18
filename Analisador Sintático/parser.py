import json
import sys
import subprocess


# Obtenção de tokens a partir da saída do scanner para determinado código c.
def obter_tokens(nome_arquivo):
    resultado = subprocess.run(
        [sys.executable, "scanner.py", nome_arquivo],
        capture_output=True,
        text=True
    )

    tokens = []
    # Cada linha do json vira um dicionário da lista em python
    for linha in resultado.stdout.splitlines():
        if linha.strip():
            tokens.append(json.loads(linha))

    return tokens


class ErroSintatico(Exception):
    pass


TIPOS = ("INT", "FLOAT", "BOOL", "CHAR", "VOID")


class Parser:

    def __init__(self, tokens):
        self.tokens = tokens
        self.posicao = 0

    # Utilidades de navegação

    def token_atual(self):
        if self.posicao < len(self.tokens):
            return self.tokens[self.posicao]
        return self.tokens[-1]

    def tipo_atual(self):
        return self.token_atual()["token"]

    def checar(self, *tipos):
        return self.tipo_atual() in tipos

    def avancar(self):
        token = self.token_atual()
        if self.posicao < len(self.tokens) - 1:
            self.posicao += 1
        return token

    def consumir(self, tipo_esperado):
        if self.tipo_atual() != tipo_esperado:
            self.erro(f"esperado {tipo_esperado}")
        return self.avancar()

    def erro(self, mensagem):
        token = self.token_atual()
        raise ErroSintatico(
            f"{mensagem}, encontrado {token['token']} "
            f"('{token['lexeme']}') na linha {token['line']}, "
            f"coluna {token['column']}"
        )

    # Programa / Declarações globais 

    def parse_programa(self):
        declaracoes = []

        while not self.checar("EOF"):
            declaracoes.append(self.parse_declaracao_global())

        return "Program(" + ", ".join(declaracoes) + ")"

    def parse_declaracao_global(self):
        if self.checar(*TIPOS):
            tipo = self.avancar()["lexeme"]
            nome = self.consumir("IDENT")["lexeme"]

            if self.checar("LPAREN"):
                return self.parse_funcao(tipo, nome)

            return self.parse_var_decl_resto(tipo, nome)

        return self.parse_comando()

    def parse_funcao(self, tipo, nome):
        self.consumir("LPAREN")

        parametros = []
        if not self.checar("RPAREN"):
            parametros.append(self.parse_parametro())
            while self.checar("COMMA"):
                self.avancar()
                parametros.append(self.parse_parametro())

        self.consumir("RPAREN")

        # A gramática não admite protótipo (função sem corpo);
        # o corpo em bloco é sempre obrigatório.
        bloco = self.parse_bloco()

        parametros_txt = ",".join(parametros)
        return f"Function({tipo} {nome}({parametros_txt}) {bloco})"

    def parse_parametro(self):
        if not self.checar(*TIPOS):
            self.erro("esperado tipo de parâmetro")

        tipo = self.avancar()["lexeme"]
        nome = self.consumir("IDENT")["lexeme"]

        return f"{tipo} {nome}"

    def parse_var_decl_resto(self, tipo, nome):
        tamanho = None
        if self.checar("LBRACKET"):
            self.avancar()
            tamanho = self.parse_expressao()
            self.consumir("RBRACKET")

        inicializador = None
        if self.checar("ASSIGN"):
            self.avancar()
            inicializador = self.parse_expressao()

        self.consumir("SEMICOLON")

        texto = f"{tipo} {nome}"
        if tamanho is not None:
            texto += f" size={tamanho}"
        if inicializador is not None:
            texto += f"={inicializador}"

        return f"VarDecl({texto})"

    #  Comandos 

    def parse_bloco(self):
        self.consumir("LBRACE")

        comandos = []
        while not self.checar("RBRACE"):
            comandos.append(self.parse_comando())

        self.consumir("RBRACE")

        return "Block(" + ", ".join(comandos) + ")"

    def parse_comando(self):
        if self.checar(*TIPOS):
            tipo = self.avancar()["lexeme"]
            nome = self.consumir("IDENT")["lexeme"]
            return self.parse_var_decl_resto(tipo, nome)

        if self.checar("IF"):
            return self.parse_if()

        if self.checar("WHILE"):
            return self.parse_while()

        if self.checar("RETURN"):
            return self.parse_return()

        if self.checar("LBRACE"):
            return self.parse_bloco()

        return self.parse_expr_stmt()

    def parse_if(self):
        self.consumir("IF")
        self.consumir("LPAREN")
        condicao = self.parse_expressao()
        self.consumir("RPAREN")

        entao = self.parse_comando()

        senao = "NULL"
        if self.checar("ELSE"):
            self.avancar()
            senao = self.parse_comando()

        return f"If({condicao},{entao},{senao})"

    def parse_while(self):
        self.consumir("WHILE")
        self.consumir("LPAREN")
        condicao = self.parse_expressao()
        self.consumir("RPAREN")

        corpo = self.parse_comando()

        return f"While({condicao},{corpo})"

    def parse_return(self):
        self.consumir("RETURN")

        if self.checar("SEMICOLON"):
            self.avancar()
            return "Return(NULL)"

        expressao = self.parse_expressao()
        self.consumir("SEMICOLON")

        return f"Return({expressao})"

    def parse_expr_stmt(self):
        expressao = self.parse_expressao()
        self.consumir("SEMICOLON")

        return f"ExprStmt({expressao})"

    # Expressões (Precedência crescente) 

    def parse_expressao(self):
        return self.parse_atribuicao()

    def parse_atribuicao(self):
        esquerda = self.parse_or()

        if self.checar("ASSIGN"):
            if not (esquerda.startswith("Id(") or esquerda.startswith("Index(")):
                self.erro("alvo de atribuição inválido")

            self.avancar()
            direita = self.parse_atribuicao()
            return f"Assign({esquerda},{direita})"

        return esquerda

    def parse_or(self):
        esquerda = self.parse_and()
        while self.checar("OR"):
            operador = self.avancar()["lexeme"]
            direita = self.parse_and()
            esquerda = f"Binary({operador},{esquerda},{direita})"
        return esquerda

    def parse_and(self):
        esquerda = self.parse_igualdade()
        while self.checar("AND"):
            operador = self.avancar()["lexeme"]
            direita = self.parse_igualdade()
            esquerda = f"Binary({operador},{esquerda},{direita})"
        return esquerda

    def parse_igualdade(self):
        esquerda = self.parse_relacional()
        while self.checar("EQ", "NE"):
            operador = self.avancar()["lexeme"]
            direita = self.parse_relacional()
            esquerda = f"Binary({operador},{esquerda},{direita})"
        return esquerda

    def parse_relacional(self):
        esquerda = self.parse_aditiva()
        while self.checar("LT", "GT", "LE", "GE"):
            operador = self.avancar()["lexeme"]
            direita = self.parse_aditiva()
            esquerda = f"Binary({operador},{esquerda},{direita})"
        return esquerda

    def parse_aditiva(self):
        esquerda = self.parse_multiplicativa()
        while self.checar("PLUS", "MINUS"):
            operador = self.avancar()["lexeme"]
            direita = self.parse_multiplicativa()
            esquerda = f"Binary({operador},{esquerda},{direita})"
        return esquerda

    def parse_multiplicativa(self):
        esquerda = self.parse_unaria()
        while self.checar("STAR", "SLASH", "PERCENT"):
            operador = self.avancar()["lexeme"]
            direita = self.parse_unaria()
            esquerda = f"Binary({operador},{esquerda},{direita})"
        return esquerda

    def parse_unaria(self):
        if self.checar("MINUS", "NOT"):
            operador = self.avancar()["lexeme"]
            operando = self.parse_unaria()
            return f"Unary({operador},{operando})"
        return self.parse_pos_fixo()

    def parse_pos_fixo(self):
        expressao = self.parse_primario()

        while True:
            if self.checar("LPAREN"):
                self.avancar()
                argumentos = []
                if not self.checar("RPAREN"):
                    argumentos.append(self.parse_expressao())
                    while self.checar("COMMA"):
                        self.avancar()
                        argumentos.append(self.parse_expressao())
                self.consumir("RPAREN")

                if argumentos:
                    expressao = f"Call({expressao}," + ",".join(argumentos) + ")"
                else:
                    expressao = f"Call({expressao})"

            elif self.checar("LBRACKET"):
                self.avancar()
                indice = self.parse_expressao()
                self.consumir("RBRACKET")
                expressao = f"Index({expressao},{indice})"

            else:
                break

        return expressao

    def parse_primario(self):
        token = self.token_atual()
        tipo = token["token"]

        if tipo == "IDENT":
            self.avancar()
            return f"Id({token['lexeme']})"

        if tipo == "INT_LIT":
            self.avancar()
            return f"Lit(int,{token['lexeme']})"

        if tipo == "FLOAT_LIT":
            self.avancar()
            return f"Lit(real,{token['lexeme']})"

        if tipo == "TRUE":
            self.avancar()
            return "Lit(bool,true)"

        if tipo == "FALSE":
            self.avancar()
            return "Lit(bool,false)"

        if tipo == "STRING_LIT":
            self.avancar()
            return f"Lit(string,{token['lexeme']})"

        if tipo == "CHAR_LIT":
            self.avancar()
            return f"Lit(char,{token['lexeme']})"

        if tipo == "LPAREN":
            self.avancar()
            expressao = self.parse_expressao()
            self.consumir("RPAREN")
            return expressao

        self.erro("esperado identificador, literal ou '('")


MENSAGEM_REJEICAO = "NÃO HÁ AST: o parser deve rejeitar a entrada."

# Função principal do parser, que recebe o nome do arquivo de entrada como argumento.
def main():
    if len(sys.argv) != 2:
        print("Uso: python parser.py <arquivo.c>", file=sys.stderr)
        return 2

    nome_arquivo = sys.argv[1]

    try:
        tokens = obter_tokens(nome_arquivo)

        if not tokens or tokens[-1].get("token") != "EOF":
            raise ErroSintatico("tokens inválidos retornados pelo scanner")

        parser = Parser(tokens)
        ast = parser.parse_programa()

        print(ast)
        return 0

    except Exception:
        # Qualquer falha de sintaxe (ou token inesperado) rejeita a entrada.
        print(MENSAGEM_REJEICAO)
        return 1


if __name__ == "__main__":
    sys.exit(main())
