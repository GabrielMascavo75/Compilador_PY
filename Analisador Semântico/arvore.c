/*
 * arvore.c — Construção dos nós da árvore sintática.
 */
#include "util.h"
#include "arvore.h"

#include <string.h>

No *novo_no(TipoNo tipo_no, int linha, int coluna)
{
    No *no = xmalloc(sizeof *no);
    memset(no, 0, sizeof *no);
    no->tipo_no = tipo_no;
    no->linha = linha;
    no->coluna = coluna;
    return no;
}

void lista_adicionar(Lista *l, No *no)
{
    if (l->n == l->cap) {
        l->cap = l->cap ? l->cap * 2 : 8;
        l->itens = xrealloc(l->itens, (size_t)l->cap * sizeof *l->itens);
    }
    l->itens[l->n++] = no;
}
