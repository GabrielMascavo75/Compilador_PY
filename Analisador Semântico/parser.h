/*
 * parser.h — Parser descendente recursivo.
 */
#ifndef PARSER_H
#define PARSER_H

#include "arvore.h"

/* Mensagem do erro sintático, quando analisar_sintaxe() devolve NULL. */
extern char *msg_erro_sintatico;

/* Constrói a árvore a partir dos tokens do scanner; NULL em caso de erro. */
No *analisar_sintaxe(void);

#endif
