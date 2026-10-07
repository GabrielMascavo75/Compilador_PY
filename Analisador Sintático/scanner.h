#ifndef SCANNER_H
#define SCANNER_H
#include <stddef.h>

typedef enum { T_INT,T_FLOAT,T_BOOL,T_CHAR,T_VOID,T_IF,T_ELSE,T_WHILE,T_FOR,T_RETURN,T_BREAK,T_CONTINUE,T_TRUE,T_FALSE,T_PRINT,T_READ,T_IDENT,T_FLOAT_LIT,T_INT_LIT,T_STRING_LIT,T_CHAR_LIT,T_EQ,T_NE,T_LE,T_GE,T_AND,T_OR,T_PLUS,T_MINUS,T_STAR,T_SLASH,T_PERCENT,T_LT,T_GT,T_NOT,T_ASSIGN,T_LPAREN,T_RPAREN,T_LBRACE,T_RBRACE,T_LBRACKET,T_RBRACKET,T_SEMICOLON,T_COMMA,T_DOT,T_EOF } TokenType;

typedef struct { TokenType tipo; char *lexema; int linha,coluna; long long ival; double fval; } Token;
typedef struct { Token *v; size_t n,cap,pos; int erro; char *erro_msg; int erro_linha,erro_coluna; } TokenVec;
const char *token_nome(TokenType t);
int scanner_ler(const char *fonte, TokenVec *out);
void tokens_free(TokenVec *v);
#endif
