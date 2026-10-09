/*
 * minic.c — Compilador MINIC 
 */
#include "util.h"
#include "scanner.h"
#include "parser.h"
#include "semantic.h"

#ifndef COMPILACAO_SEPARADA
#include "util.c"
#include "scanner.c"
#include "arvore.c"
#include "parser.c"
#include "semantic.c"
#endif

#include <stdio.h>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif


#define SEPARADOR "\r\n"

static void emitir_linha(const char *linha, int primeira)
{
    if (!primeira)
        fputs(SEPARADOR, stdout);
    fputs(linha, stdout);
}

int main(int argc, char **argv)
{
    No *ast;
    int i;

#ifdef _WIN32
    _setmode(_fileno(stdout), _O_BINARY);
#endif

    if (argc != 2) {
        fprintf(stderr, "Uso: %s <arquivo.c>\n", argv[0]);
        return 1;
    }

    if (!ler_fonte(argv[1])) {
        fprintf(stderr, "Arquivo não encontrado: %s\n", argv[1]);
        return 1;
    }

    if (!utf8_valido((const unsigned char *)fonte, tam_fonte)) {
        emitir_linha("Erro léxico: tokens inválidos retornados pelo scanner", 1);
        return 2;
    }

    if (!lexer()) {
        emitir_linha(fmt("Erro léxico: %s", msg_erro_lexico), 1);
        return 2;
    }

    ast = analisar_sintaxe();
    if (!ast) {
        emitir_linha(fmt("Erro sintático: %s", msg_erro_sintatico), 1);
        return 3;
    }

    analisar_semantica(ast);

    for (i = 0; i < n_erros; i++)
        emitir_linha(fmt("%s — linha %d, coluna %d: %s", erros[i].codigo,
                         erros[i].linha, erros[i].coluna, erros[i].mensagem),
                     i == 0);

    emitir_linha(fmt("Análise semântica concluída: %s; %s.",
                     plural(n_erros, "erro", "erros"),
                     n_erros == 0 ? "programa aceito" : "programa rejeitado"),
                 n_erros == 0);
    fflush(stdout);

    return n_erros == 0 ? 0 : 4;
}
