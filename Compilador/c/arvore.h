/*
 * arvore.h — Nós da árvore sintática.
 */
#ifndef ARVORE_H
#define ARVORE_H

#include <stddef.h>

typedef enum {
    /* Programa e declarações */
    N_PROGRAMA, N_FUNCAO, N_PARAMETRO, N_DECLVAR, N_LISTADECL,
    /* Comandos */
    N_BLOCO, N_IF, N_WHILE, N_FOR, N_RETURN, N_BREAK, N_CONTINUE,
    N_PRINT, N_READ, N_EXPRSTMT,
    /* Expressões */
    N_ATRIBUICAO, N_BINARIA, N_UNARIA, N_CHAMADA, N_INDICE,
    N_IDENTIFICADOR, N_LITERAL
} TipoNo;

typedef enum { B_INT, B_FLOAT, B_BOOL, B_CHAR, B_VOID, B_STRING, B_ERRO } Base;

typedef struct {
    Base base;
    int vetor;
} Tipo;

/* Valor de uma expressão constante */
typedef struct {
    int ok;          /* 0 = None */
    int inteiro;     /* 1 = int, 0 = float */
    long long i;
    double r;
} Const;

typedef struct Simbolo Simbolo;
typedef struct No No;

typedef struct {
    No **itens;
    int n, cap;
} Lista;

/* Um único tipo de struct para todos os nós; cada tipo de nó usa só os
 * campos que lhe dizem respeito */
struct No {
    TipoNo tipo_no;

    /* Posição de início, usada nos diagnósticos. */
    int linha, coluna;
    /* Trecho do fonte (bytes, fim exclusivo); preenchido para expressões. */
    size_t ini, fim;
    int tem_trecho;

    /* Filhos */
    No *condicao;        /* If, While, For */
    No *entao, *senao;   /* If */
    No *corpo;           /* While, For, Funcao */
    No *inicio, *passo;  /* For */
    No *expressao;       /* Return, ExprStmt (NULL = ausente) */
    No *argumento;       /* Print */
    No *alvo, *valor;    /* Atribuicao (alvo também em Read) */
    No *esquerda, *direita;  /* Binaria */
    No *operando;        /* Unaria */
    No *funcao;          /* Chamada */
    No *vetor, *indice;  /* Indice */
    No *tamanho, *inicializador;  /* DeclVar */
    Lista lista;         /* Programa, ListaDecl, Bloco: itens;
                            Funcao: parâmetros; Chamada: argumentos */

    /* Dados */
    const char *nome;         /* Identificador, DeclVar, Parametro, Funcao */
    const char *operador;     /* Binaria, Unaria */
    const char *tipo_base;    /* DeclVar, Parametro, Funcao (retorno) */
    int eh_vetor;             /* Parametro */
    int linha_nome, coluna_nome;   /* Funcao: posição do nome */
    const char *tipo_literal; /* Literal: int, real, bool, char, string */
    const char *lexema;       /* Literal */
    Const valor_lit;          /* Literal int/real */

    /* Anotações da análise semântica */
    Tipo tipo;
    Simbolo *simbolo;
};

No *novo_no(TipoNo tipo_no, int linha, int coluna);
void lista_adicionar(Lista *l, No *no);

#endif
