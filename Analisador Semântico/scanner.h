/*
 * scanner.h — Analisador léxico.
 */
#ifndef SCANNER_H
#define SCANNER_H

#include <stddef.h>

typedef enum {
    /* Palavras reservadas — na mesma ordem de PALAVRAS, abaixo */
    T_INT, T_FLOAT, T_BOOL, T_CHAR, T_VOID, T_IF, T_ELSE, T_WHILE, T_FOR,
    T_RETURN, T_BREAK, T_CONTINUE, T_TRUE, T_FALSE, T_PRINT, T_READ,
    /* Literais e identificadores */
    T_STRING_LIT, T_CHAR_LIT, T_IDENT, T_FLOAT_LIT, T_INT_LIT,
    /* Operadores */
    T_EQ, T_NE, T_LE, T_GE, T_AND, T_OR,
    T_PLUS, T_MINUS, T_STAR, T_SLASH, T_PERCENT,
    T_LT, T_GT, T_NOT, T_ASSIGN,
    /* Delimitadores */
    T_LPAREN, T_RPAREN, T_LBRACE, T_RBRACE, T_LBRACKET, T_RBRACKET,
    T_SEMICOLON, T_COMMA, T_DOT,
    T_EOF,
    /* Internos do scanner: descartados, nunca chegam ao parser */
    T_IGNORAR
} TipoToken;

/* Nomes usados nas mensagens de erro sintático ("esperado SEMICOLON"). */
extern const char *NOME_TOKEN[];

typedef struct {
    TipoToken tipo;
    char *lexema;
    size_t ini, fim;      /* trecho no fonte, em bytes (fim exclusivo) */
    int linha, coluna;    /* posição de início, como no scanner.py */
} Token;

/* Código-fonte, já com quebras de linha normalizadas para "\n". */
extern char *fonte;
extern size_t tam_fonte;

/* Tokens produzidos pelo lexer (o último é sempre T_EOF). */
extern Token *tokens;
extern int n_tokens;

/* Mensagem do primeiro erro léxico, quando lexer() devolve 0. */
extern char *msg_erro_lexico;

/* Lê o arquivo; devolve 0 se ele não existir ou não for um arquivo comum. */
int ler_fonte(const char *nome);

/* Verifica se o fonte é UTF-8 válido (o scanner.py falharia ao decodificar). */
int utf8_valido(const unsigned char *s, size_t n);

/* Gera os tokens; devolve 1 em caso de sucesso ou 0 em caso de erro. */
int lexer(void);

#endif
