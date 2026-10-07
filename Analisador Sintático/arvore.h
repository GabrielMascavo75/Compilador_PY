#ifndef ARVORE_H
#define ARVORE_H
#include <stddef.h>

typedef enum { N_PROGRAMA, N_FUNCAO, N_PARAMETRO, N_DECLVAR, N_LISTADECL, N_BLOCO, N_IF, N_WHILE, N_FOR, N_RETURN, N_BREAK, N_CONTINUE, N_PRINT, N_READ, N_EXPRSTMT, N_ATRIB, N_BINARIA, N_UNARIA, N_CHAMADA, N_INDICE, N_IDENT, N_LITERAL } NodeKind;

typedef enum { L_INT, L_REAL, L_BOOL, L_CHAR, L_STRING } LiteralKind;

typedef struct No No;

typedef struct { No **v; size_t n, cap; } NoVec;

struct No {
    NodeKind kind;
    int linha, coluna;
    int fim_linha, fim_coluna;
    int tem_fim;
    int tipo_sem; /* preenchido pelo semântico */
    void *simbolo;
    union {
        struct { NoVec declaracoes; } programa;
        struct { char *tipo_retorno, *nome; NoVec parametros; No *corpo; int linha_nome, coluna_nome; } funcao;
        struct { char *tipo_base, *nome; int eh_vetor; } parametro;
        struct { char *tipo_base, *nome; No *tamanho, *inicializador; } declvar;
        struct { NoVec declaracoes; } listadecl;
        struct { NoVec comandos; } bloco;
        struct { No *condicao, *entao, *senao; } ifn;
        struct { No *condicao, *corpo; } whilen;
        struct { No *inicio, *condicao, *passo, *corpo; } forn;
        struct { No *expressao; } ret;
        struct { No *argumento; } printn;
        struct { No *alvo; } readn;
        struct { No *expressao; } exprstmt;
        struct { No *alvo, *valor; } atrib;
        struct { char *operador; No *esquerda, *direita; } bin;
        struct { char *operador; No *operando; } un;
        struct { No *funcao; NoVec argumentos; } chamada;
        struct { No *vetor, *indice; } indice;
        struct { char *nome; } ident;
        struct { LiteralKind tipo_literal; char *lexema; char *valor_str; long long valor_int; double valor_real; int valor_bool; } literal;
    } u;
};

void novec_push(NoVec *v, No *n);
No *no_novo(NodeKind kind, int linha, int coluna);
void no_marcar_fim(No *n, int linha, int coluna);
void no_free(No *n);
void ast_imprimir(const No *n);
#endif
