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
    def resultado(self):
        if self.teve_erro:
            return 3
        
        return 0

if __name__ == "__main__":

    nome_arquivo = sys.argv[1]

    tokens = obter_tokens(nome_arquivo)

    parser = Parser(tokens)

    parser.parse_tipo()

    sys.exit(parser.resultado())