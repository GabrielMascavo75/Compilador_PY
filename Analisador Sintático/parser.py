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


class Parser:

    def __init__(self, tokens):
        self.tokens = tokens
        self.posicao = 0
        self.teve_erro = False

    def token_atual(self):
        return self.tokens[self.posicao]

    def avancar(self):
        self.posicao += 1

    def consumir(self, tipo_esperado):
        token = self.token_atual()

        if token["token"] == tipo_esperado:
            self.avancar()
        else:
            erro = {
                "error": "UNEXPECTED_TOKEN",
                "lexeme": token["lexeme"],
                "line": token["line"],
                "column": token["column"]
            }

            print(json.dumps(erro, ensure_ascii=False))
            self.teve_erro = True

    def parse_tipo(self):
        token = self.token_atual()

        tipos = ["INT", "FLOAT", "BOOL", "CHAR"]

        if token["token"] in tipos:
            self.avancar()
        else:
            erro = {
                "error": "UNEXPECTED_TOKEN",
                "lexeme": token["lexeme"],
                "line": token["line"],
                "column": token["column"]
            }

            print(json.dumps(erro, ensure_ascii=False))
            self.teve_erro = True

    def parse_identificador(self):
        token = self.token_atual()

        if token["token"] == "IDENT":
            self.avancar()
        else:
            erro = {
                "error": "UNEXPECTED_TOKEN",
                "lexeme": token["lexeme"],
                "line": token["line"],
                "column": token["column"]
            }

            print(json.dumps(erro, ensure_ascii=False))
            self.teve_erro = True
    def parse_valor(self):
        token = self.token_atual()

        if token["token"] == "INT_LIT":
            self.avancar()

        elif token["token"] == "FLOAT_LIT":
            self.avancar()

        else:
            erro = {
                "error": "UNEXPECTED_TOKEN",
                "lexeme": token["lexeme"],
                "line": token["line"],
                "column": token["column"]
            }

            print(json.dumps(erro, ensure_ascii=False))
            self.teve_erro = True

    def parse_expressao(self):
        self.parse_expressao_or()

    def parse_expressao_or(self):
        self.parse_expressao_and()

        while self.token_atual()["token"] == "OR":
            self.avancar()
            self.parse_expressao_and()

    def parse_expressao_and(self):
        self.parse_expressao_igualdade()

        while self.token_atual()["token"] == "AND":
            self.avancar()
            self.parse_expressao_igualdade()

    def parse_expressao_igualdade(self):
        self.parse_expressao_relacional()

        while self.token_atual()["token"] in ["EQ", "NE"]:
            self.avancar()
            self.parse_expressao_relacional()

    def parse_expressao_relacional(self):
        self.parse_expressao_aditiva()

        while self.token_atual()["token"] in ["LT", "GT", "LE", "GE"]:
            self.avancar()
            self.parse_expressao_aditiva() 

    def parse_expressao_aditiva(self):
        self.parse_expressao_multiplicativa()

        while self.token_atual()["token"] in ["PLUS", "MINUS"]:
            self.avancar()
            self.parse_expressao_multiplicativa()

    def parse_expressao_multiplicativa(self):
        self.parse_valor()

        while self.token_atual()["token"] in ["MULT", "DIV", "MOD"]:
            self.avancar()
            self.parse_valor()


    def resultado(self):
        if self.teve_erro:
            return 3
        
        return 0

if __name__ == "__main__":

    nome_arquivo = sys.argv[1]

    tokens = obter_tokens(nome_arquivo)

    parser = Parser(tokens)

    parser.parse_tipo()

    parser.parse_identificador()

    if parser.token_atual()["token"] == "ASSIGN":
        parser.consumir("ASSIGN")
        parser.parse_expressao()

    parser.consumir("SEMICOLON")

    sys.exit(parser.resultado())
