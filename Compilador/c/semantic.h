/*
 * semantic.h — Análise semântica (equivalente ao semantic.py).
 */
#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "arvore.h"

typedef struct {
    const char *codigo;
    int linha, coluna;
    const char *mensagem;
} Diagnostico;

/* Diagnósticos encontrados, já em ordem de origem após a análise. */
extern Diagnostico *erros;
extern int n_erros;

/* Verifica nomes, escopos, tipos e regras contextuais da árvore. */
void analisar_semantica(No *programa);

/* "1 erro", "2 erros"... */
const char *plural(int n, const char *singular, const char *plural_);

#endif
