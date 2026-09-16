import json
import sys
import subprocess


def obter_tokens(nome_arquivo):
    resultado = subprocess.run(
        [sys.executable, "scanner.py", nome_arquivo],
        capture_output=True,
        text=True
    )

    tokens = []

    for linha in resultado.stdout.splitlines():
        if linha.strip():
            tokens.append(json.loads(linha))

    return tokens

class Program:
    def __init__(self):
        self.global_declarations = []
        self.function_declarations = []
        self.main_function = None

class MainFunction:
    def __init__(self):
        self.body = None

class Block:
    def __init__(self):
        self.items = []

class VarDecl:
    def __init__(self, tipo, nome, inicializacao=None, tamanho=None):
        self.tipo = tipo
        self.nome = nome
        self.inicializacao = inicializacao
        self.tamanho = tamanho

class Literal:
    def __init__(self, valor):
        self.valor = valor

class BinaryOp:
    def __init__(self, operador, esquerda, direita):
        self.operador = operador
        self.esquerda = esquerda
        self.direita = direita

class UnaryOp:
    def __init__(self, operador, operando):
        self.operador = operador
        self.operando = operando

class Variable:
    def __init__(self, nome):
        self.nome = nome

class Assignment:
    def __init__(self, alvo, valor):
        self.alvo = alvo
        self.valor = valor

class CallExpr:
    def __init__(self, funcao, argumentos):
        self.funcao = funcao
        self.argumentos = argumentos

class ArrayAccess:
    def __init__(self, array, indice):
        self.array = array
        self.indice = indice

class ReturnStmt:
    def __init__(self, expressao=None):
        self.expressao = expressao

class IfStmt:
    def __init__(self, condicao, corpo, senao=None):
        self.condicao = condicao
        self.corpo = corpo
        self.senao = senao

class WhileStmt:
    def __init__(self, condicao, corpo):
        self.condicao = condicao
        self.corpo = corpo

class ForStmt:
    def __init__(self, inicializacao, condicao, atualizacao, corpo):
        self.inicializacao = inicializacao
        self.condicao = condicao
        self.atualizacao = atualizacao
        self.corpo = corpo

class BreakStmt:
    def __init__(self):
        pass

class ContinueStmt:
    def __init__(self):
        pass

class PrintStmt:
    def __init__(self, argumento):
        self.argumento = argumento

class ReadStmt:
    def __init__(self, localizavel):
        self.localizavel = localizavel

class FunctionDecl:
    def __init__(self, tipo_retorno, nome, parametros, corpo):
        self.tipo_retorno = tipo_retorno
        self.nome = nome
        self.parametros = parametros
        self.corpo = corpo

class Parameter:
    def __init__(self, tipo, nome, array=False):
        self.tipo = tipo
        self.nome = nome
        self.array = array

class Parser:
    def __init__(self, tokens):
        self.tokens = tokens
        self.posicao = 0
        self.teve_erro = False

    # Controle dos tokens

    def token_atual(self):
        return self.tokens[self.posicao]

    def avancar(self):
        self.posicao += 1

    def proximo_token(self):
        if self.posicao + 1 < len(self.tokens):
            return self.tokens[self.posicao + 1]

        return None

    def pular_ate(self, tipos):
        while (
            self.token_atual()["token"] not in tipos
            and self.token_atual()["token"] != "EOF"
        ):
            self.avancar()

    def erro_unexpected_token(self):
        token = self.token_atual()

        erro = {
            "error": "UNEXPECTED_TOKEN",
            "lexeme": token["lexeme"],
            "line": token["line"],
            "column": token["column"]
        }

        print(json.dumps(erro, ensure_ascii=False))
        self.teve_erro = True

        self.avancar()

    def erro_main_duplicado(self):
        token = self.token_atual()

        erro = {
            "error": "DUPLICATE_MAIN",
            "lexeme": token["lexeme"],
            "line": token["line"],
            "column": token["column"]
        }

        print(json.dumps(erro, ensure_ascii=False))
        self.teve_erro = True

    def consumir(self, tipo_esperado):
        token = self.token_atual()

        if token["token"] == tipo_esperado:
            self.avancar()
        else:
            self.erro_unexpected_token()

    # Tipos e identificadores

    def parse_tipo(self):
        token = self.token_atual()
        tipos = ["INT", "FLOAT", "BOOL", "CHAR"]

        if token["token"] in tipos:
            self.avancar()
            return token["lexeme"]
        else:
            self.erro_unexpected_token()
            return None

    def parse_tipo_retorno(self):
        token = self.token_atual()

        tipos = ["INT", "FLOAT", "BOOL", "CHAR", "VOID"]

        if token["token"] in tipos:
            self.avancar()
            return token["lexeme"]

        self.erro_unexpected_token()
        return None

    def parse_identificador(self):
        token = self.token_atual()

        if token["token"] != "IDENT":
            self.erro_unexpected_token()
            return None

        self.avancar()
        return token["lexeme"]

    def parse_declaracao_global(self):
        tipo = self.parse_tipo()

        declaracoes = []

        nome, inicializacao = self.parse_declarador()

        if nome is not None:
            declaracoes.append(VarDecl(tipo, nome, inicializacao))

        while self.token_atual()["token"] == "COMMA":
            self.avancar()

            nome, inicializacao = self.parse_declarador()

            if nome is not None:
                declaracoes.append(VarDecl(tipo, nome, inicializacao))

        self.consumir("SEMICOLON")

        return declaracoes

    def parse_declaracao_funcao(self):
        tipo_retorno = self.parse_tipo_retorno()

        nome = self.parse_identificador()

        self.consumir("LPAREN")

        parametros = []

        if self.token_atual()["token"] != "RPAREN":
            parametros = self.parse_parametros()

        self.consumir("RPAREN")

        corpo = self.parse_bloco()

        return FunctionDecl(
            tipo_retorno,
            nome,
            parametros,
            corpo
        )

    def parse_parametro(self):

        self.parse_tipo()
        self.parse_identificador()

        if self.token_atual()["token"] == "LBRACKET":
            self.avancar()
            self.consumir("RBRACKET")

    def parse_parametros(self):
        parametros = []

        tipo = self.parse_tipo()
        nome = self.parse_identificador()

        array = False

        if self.token_atual()["token"] == "LBRACKET":
            self.avancar()
            self.consumir("RBRACKET")
            array = True

        parametros.append(Parameter(tipo, nome, array))

        while self.token_atual()["token"] == "COMMA":
            self.avancar()

            tipo = self.parse_tipo()
            nome = self.parse_identificador()

            array = False

            if self.token_atual()["token"] == "LBRACKET":
                self.avancar()
                self.consumir("RBRACKET")
                array = True

            parametros.append(Parameter(tipo, nome, array))

        return parametros

    def parse_primario(self):
        token = self.token_atual()

        if token["token"] == "IDENT":
            self.avancar()
            return Variable(token["lexeme"])

        elif token["token"] == "INT_LIT":
            self.avancar()
            return Literal(token["attribute"])

        elif token["token"] == "FLOAT_LIT":
            self.avancar()
            return Literal(token["attribute"])

        elif token["token"] == "TRUE":
            self.avancar()
            return Literal(True)

        elif token["token"] == "FALSE":
            self.avancar()
            return Literal(False)

        elif token["token"] == "CHAR_LIT":
            self.avancar()
            return Literal(token["attribute"])

        elif token["token"] == "LPAREN":
            self.avancar()
            expressao = self.parse_expressao()
            self.consumir("RPAREN")
            return expressao

        else:
            self.erro_unexpected_token()
            return None

    def parse_argumentos(self):
        argumentos = []

        argumentos.append(self.parse_expressao())

        while self.token_atual()["token"] == "COMMA":
            self.avancar()
            argumentos.append(self.parse_expressao())

        return argumentos

    def parse_expressao_posfixa(self):
        expr = self.parse_primario()

        while self.token_atual()["token"] in ["LBRACKET", "LPAREN"]:

            if self.token_atual()["token"] == "LBRACKET":
                self.avancar()

                indice = self.parse_expressao()

                self.consumir("RBRACKET")

                expr = ArrayAccess(expr, indice)

            elif self.token_atual()["token"] == "LPAREN":
                self.avancar()

                argumentos = []

                if self.token_atual()["token"] != "RPAREN":
                    argumentos = self.parse_argumentos()

                self.consumir("RPAREN")

                expr = CallExpr(expr, argumentos)

        return expr
    
    def parse_expressao_unaria(self):
        token = self.token_atual()

        if token["token"] in ["MINUS", "NOT"]:
            operador = token["lexeme"]
            self.avancar()

            operando = self.parse_expressao_unaria()

            return UnaryOp(operador, operando)

        return self.parse_expressao_posfixa()

    def parse_expressao_multiplicativa(self):
        esquerda = self.parse_expressao_unaria()

        while self.token_atual()["token"] in ["STAR", "DIV", "MOD"]:
            operador = self.token_atual()["lexeme"]

            self.avancar()

            direita = self.parse_expressao_unaria()

            esquerda = BinaryOp(operador, esquerda, direita)

        return esquerda

    def parse_expressao_aditiva(self):
        esquerda = self.parse_expressao_multiplicativa()

        while self.token_atual()["token"] in ["PLUS", "MINUS"]:
            operador = self.token_atual()["lexeme"]
            self.avancar()

            direita = self.parse_expressao_multiplicativa()

            esquerda = BinaryOp(operador, esquerda, direita)

        return esquerda

    def parse_expressao_relacional(self):
        esquerda = self.parse_expressao_aditiva()

        while self.token_atual()["token"] in ["LT", "GT", "LE", "GE"]:
            operador = self.token_atual()["lexeme"]
            self.avancar()

            direita = self.parse_expressao_aditiva()

            esquerda = BinaryOp(operador, esquerda, direita)

        return esquerda

    def parse_expressao_igualdade(self):
        esquerda = self.parse_expressao_relacional()

        while self.token_atual()["token"] in ["EQ", "NE"]:
            operador = self.token_atual()["lexeme"]
            self.avancar()

            direita = self.parse_expressao_relacional()

            esquerda = BinaryOp(operador, esquerda, direita)

        return esquerda

    def parse_expressao_and(self):
        esquerda = self.parse_expressao_igualdade()

        while self.token_atual()["token"] == "AND":
            operador = self.token_atual()["lexeme"]
            self.avancar()

            direita = self.parse_expressao_igualdade()

            esquerda = BinaryOp(operador, esquerda, direita)

        return esquerda

    def parse_expressao_or(self):
        esquerda = self.parse_expressao_and()

        while self.token_atual()["token"] == "OR":
            operador = self.token_atual()["lexeme"]
            self.avancar()

            direita = self.parse_expressao_and()

            esquerda = BinaryOp(operador, esquerda, direita)

        return esquerda

    def parse_localizavel(self):
        token = self.token_atual()

        if token["token"] != "IDENT":
            self.erro_unexpected_token()
            return None

        alvo = Variable(token["lexeme"])
        self.avancar()

        if self.token_atual()["token"] == "LBRACKET":
            self.avancar()
            indice = self.parse_expressao()
            self.consumir("RBRACKET")

            return ArrayAccess(alvo, indice)

        return alvo


    def parse_atribuicao(self):
        if self.token_atual()["token"] == "IDENT":
            posicao_original = self.posicao

            alvo = self.parse_localizavel()

            if self.token_atual()["token"] == "ASSIGN":
                self.consumir("ASSIGN")
                valor = self.parse_atribuicao()

                return Assignment(alvo, valor)

            self.posicao = posicao_original

        return self.parse_expressao_or()


    def parse_expressao(self):
        return self.parse_atribuicao()


    def parse_declarador(self):
        token = self.token_atual()

        if token["token"] != "IDENT":
            self.erro_unexpected_token()
            return None, None, None

        nome = token["lexeme"]
        self.avancar()

        inicializacao = None
        tamanho = None

        if self.token_atual()["token"] == "LBRACKET":
            self.avancar()

            tamanho = self.token_atual()["attribute"]
            self.consumir("INT_LIT")

            self.consumir("RBRACKET")

        elif self.token_atual()["token"] == "ASSIGN":
            self.avancar()

            inicializacao = self.parse_expressao()

        return nome, inicializacao, tamanho
    # Declaração local

    def parse_declaracao_local(self):
        tipo = self.parse_tipo()

        declaracoes = []

        nome, inicializacao, tamanho = self.parse_declarador()

        if nome is not None:
            declaracoes.append(
                VarDecl(tipo, nome, inicializacao, tamanho)
            )

        while self.token_atual()["token"] == "COMMA":
            self.avancar()

            nome, inicializacao, tamanho = self.parse_declarador()

            if nome is not None:
                declaracoes.append(
                    VarDecl(tipo, nome, inicializacao, tamanho)
                )

        self.consumir("SEMICOLON")

        return declaracoes


    def parse_return(self):
        self.consumir("RETURN")

        expressao = None

        if self.token_atual()["token"] != "SEMICOLON":
            expressao = self.parse_expressao()

        self.consumir("SEMICOLON")

        return ReturnStmt(expressao)


    def parse_break(self):
        self.consumir("BREAK")
        self.consumir("SEMICOLON")

        return BreakStmt()


    def parse_continue(self):
        self.consumir("CONTINUE")
        self.consumir("SEMICOLON")

        return ContinueStmt()


    def parse_comando_expressao(self):
        if self.token_atual()["token"] != "SEMICOLON":
            self.parse_expressao()

        self.consumir("SEMICOLON")

    def parse_if(self):
        self.consumir("IF")
        self.consumir("LPAREN")

        condicao = self.parse_expressao()

        self.consumir("RPAREN")

        corpo = self.parse_comando()

        senao = None

        if self.token_atual()["token"] == "ELSE":
            self.avancar()
            senao = self.parse_comando()

        return IfStmt(condicao, corpo, senao)

    def parse_while(self):
        self.consumir("WHILE")
        self.consumir("LPAREN")

        condicao = self.parse_expressao()

        self.consumir("RPAREN")

        corpo = self.parse_comando()

        return WhileStmt(condicao, corpo)

    def parse_for(self):
        self.consumir("FOR")
        self.consumir("LPAREN")

        inicializacao = None
        condicao = None
        atualizacao = None

        if self.token_atual()["token"] != "SEMICOLON":
            inicializacao = self.parse_expressao()

        self.consumir("SEMICOLON")

        if self.token_atual()["token"] != "SEMICOLON":
            condicao = self.parse_expressao()

        self.consumir("SEMICOLON")

        if self.token_atual()["token"] != "RPAREN":
            atualizacao = self.parse_expressao()

        self.consumir("RPAREN")

        corpo = self.parse_comando()

        return ForStmt(inicializacao, condicao, atualizacao, corpo)

    def parse_print(self):
        self.consumir("PRINT")
        self.consumir("LPAREN")

        argumento = self.parse_expressao()

        self.consumir("RPAREN")
        self.consumir("SEMICOLON")

        return PrintStmt(argumento)

    def parse_read(self):
        self.consumir("READ")
        self.consumir("LPAREN")

        localizavel = self.parse_localizavel()

        self.consumir("RPAREN")
        self.consumir("SEMICOLON")

        return ReadStmt(localizavel)

    def parse_comando(self):
        token = self.token_atual()["token"]

        if token == "IF":
            return self.parse_if()

        elif token == "WHILE":
            return self.parse_while()

        elif token == "FOR":
            return self.parse_for()

        elif token == "RETURN":
            return self.parse_return()

        elif token == "BREAK":
            return self.parse_break()

        elif token == "CONTINUE":
            return self.parse_continue()

        elif token == "PRINT":
            return self.parse_print()

        elif token == "READ":
            return self.parse_read()

        elif token == "LBRACE":
            self.parse_bloco()

        else:
            self.parse_comando_expressao()
    

    # Bloco

    def parse_bloco(self):
            bloco = Block()

            self.consumir("LBRACE")
    
            while self.token_atual()["token"] != "RBRACE" and \
                self.token_atual()["token"] != "EOF":
    
                token = self.token_atual()["token"]
    
                if token in ["INT", "FLOAT", "BOOL", "CHAR"]:
                    declaracoes = self.parse_declaracao_local()
                    bloco.items.extend(declaracoes)
                else:
                    comando = self.parse_comando()

                    if comando is not None:
                        bloco.items.append(comando)
    
            self.consumir("RBRACE")

            return bloco


    def parse_funcao_main(self):

        main = MainFunction()

        # tipo de retorno
        self.parse_tipo()

        # nome da função
        self.parse_identificador()

        # (
        self.consumir("LPAREN")

        self.consumir("RPAREN")

        # corpo
        main.body = self.parse_bloco()

        return main

    # Programa

    def parse_programa(self):
        programa = Program()
        encontrou_main = False

        while self.token_atual()["token"] != "EOF":

            if self.token_atual()["token"] not in ["INT", "FLOAT", "BOOL", "CHAR", "VOID"]:
                self.erro_unexpected_token()
                self.avancar()
                continue

            # Precisamos olhar o identificador
            if self.proximo_token() is None:
                self.erro_unexpected_token()
                self.avancar()
                continue

            # Verifica se é realmente um identificador
            if self.proximo_token()["token"] != "IDENT":
                self.erro_unexpected_token()
                self.avancar()
                continue

            nome = self.proximo_token()["lexeme"]

            # Verifica o token depois do identificador
            if self.posicao + 2 >= len(self.tokens):
                self.erro_unexpected_token()
                self.avancar()
                continue

            token_depois_identificador = self.tokens[self.posicao + 2]["token"]

            # Se for main
            if nome == "main" and token_depois_identificador == "LPAREN":
                if encontrou_main:
                    self.erro_main_duplicado()

                    self.pular_ate(["RBRACE"])

                    if self.token_atual()["token"] == "RBRACE":
                        self.avancar()

                    continue

                encontrou_main = True
                programa.main_function = self.parse_funcao_main()

            # Se for uma função
            elif token_depois_identificador == "LPAREN":
                funcao = self.parse_declaracao_funcao()

                if funcao is not None:
                    programa.function_declarations.append(funcao)

            # Se for uma declaração global
            elif token_depois_identificador in ["ASSIGN", "SEMICOLON"]:
                declaracoes = self.parse_declaracao_global()

                if declaracoes:
                    programa.global_declarations.extend(declaracoes)

            else:
                self.erro_unexpected_token()
                self.avancar()

        # O programa precisa ter um main
        if not encontrou_main:
            token = self.token_atual()

            erro = {
                "error": "MISSING_MAIN",
                "lexeme": "",
                "line": token["line"],
                "column": token["column"]
            }

            print(json.dumps(erro, ensure_ascii=False))
            self.teve_erro = True
        return programa

    # Resultado

    def resultado(self):
        if self.teve_erro:
            return 3

        return 0


if __name__ == "__main__":

    if len(sys.argv) != 2:
        print("Uso: python parser.py <arquivo.c>")
        sys.exit(1)

    nome_arquivo = sys.argv[1]

    tokens = obter_tokens(nome_arquivo)

    parser = Parser(tokens)

    parser.parse_programa()

    sys.exit(parser.resultado())

