
# Driver do compilador MINIC (etapa semântica).

import os
import sys

from parser import analisar_sintaxe, ErroLexico, ErroSintatico
from semantic import semantic, plural

# Os gabaritos usam CRLF entre linhas e não têm quebra no final do arquivo;
# o diff do script de testes compara byte a byte.
SEPARADOR = "\r\n"


def emitir(linhas):
    # Escreve bytes UTF-8 diretamente: evita que o Python traduza "\n"
    # (no Windows) ou use outra codificação conforme o locale.
    sys.stdout.buffer.write(SEPARADOR.join(linhas).encode("utf-8"))
    sys.stdout.flush()


def main():
    if len(sys.argv) != 2:
        print("Uso: python3 minic.py <arquivo.c>", file=sys.stderr)
        return 1

    nome_arquivo = sys.argv[1]
    if not os.path.isfile(nome_arquivo):
        print(f"Arquivo não encontrado: {nome_arquivo}", file=sys.stderr)
        return 1

    try:
        # estrito=False: "3 = n" chega à análise semântica (SEM013).
        ast = analisar_sintaxe(nome_arquivo, estrito=False)
    except ErroLexico as e:
        emitir([f"Erro léxico: {e}"])
        return 2
    except ErroSintatico as e:
        emitir([f"Erro sintático: {e}"])
        return 3

    # Mesma leitura do scanner (texto UTF-8), para as colunas baterem.
    with open(nome_arquivo, encoding="utf-8") as arquivo:
        fonte = arquivo.read()

    erros = semantic(fonte).analisar(ast)

    linhas = [str(e) for e in erros]
    situacao = "programa aceito" if not erros else "programa rejeitado"
    linhas.append(f"Análise semântica concluída: "
                  f"{plural(len(erros), 'erro', 'erros')}; {situacao}.")
    emitir(linhas)

    return 0 if not erros else 4


if __name__ == "__main__":
    sys.exit(main())
