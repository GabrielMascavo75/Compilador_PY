#ifndef SEMANTIC_H
#define SEMANTIC_H
#include "arvore.h"

typedef enum { TY_INT,TY_FLOAT,TY_BOOL,TY_CHAR,TY_VOID,TY_STRING,TY_ERRO } BaseType;
typedef struct { BaseType base; int vetor; } Tipo;
typedef struct Simbolo Simbolo;
typedef struct Diagnostico { char *codigo,*mensagem; int linha,coluna; } Diagnostico;
typedef struct { Diagnostico *v; size_t n,cap; } DiagnosticoVec;
typedef struct Semantic Semantic;
Semantic *semantic_new(const char *fonte);
void semantic_free(Semantic *s);
DiagnosticoVec *semantic_analisar(Semantic*s,No*programa);
const char *tipo_str(Tipo t);
#endif
