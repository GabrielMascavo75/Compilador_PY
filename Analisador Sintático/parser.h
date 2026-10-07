#ifndef PARSER_H
#define PARSER_H
#include "arvore.h"
#include "scanner.h"
typedef struct { const char *fonte; TokenVec toks; size_t p; int estrito; char *erro; } Parser;
int parser_init(Parser *p,const char *fonte,int estrito);
No *analisar_sintaxe(Parser *p);
void parser_free(Parser *p);
#endif
