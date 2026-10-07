#include "arvore.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void novec_push(NoVec *v, No *n){ if(v->n==v->cap){v->cap=v->cap?v->cap*2:4; v->v=realloc(v->v,v->cap*sizeof(No*)); if(!v->v){perror("realloc");exit(1);}} v->v[v->n++]=n; }
No *no_novo(NodeKind k,int l,int c){ No*n=calloc(1,sizeof(*n)); if(!n){perror("calloc");exit(1);} n->kind=k;n->linha=l;n->coluna=c;return n; }
void no_marcar_fim(No*n,int l,int c){n->fim_linha=l;n->fim_coluna=c;n->tem_fim=1;}
static void freevec(NoVec*v){for(size_t i=0;i<v->n;i++)no_free(v->v[i]);free(v->v);}
void no_free(No*n){if(!n)return; switch(n->kind){
case N_PROGRAMA: freevec(&n->u.programa.declaracoes);break;
case N_FUNCAO: free(n->u.funcao.tipo_retorno);free(n->u.funcao.nome);freevec(&n->u.funcao.parametros);no_free(n->u.funcao.corpo);break;
case N_PARAMETRO: free(n->u.parametro.tipo_base);free(n->u.parametro.nome);break;
case N_DECLVAR: free(n->u.declvar.tipo_base);free(n->u.declvar.nome);no_free(n->u.declvar.tamanho);no_free(n->u.declvar.inicializador);break;
case N_LISTADECL: freevec(&n->u.listadecl.declaracoes);break;
case N_BLOCO: freevec(&n->u.bloco.comandos);break;
case N_IF: no_free(n->u.ifn.condicao);no_free(n->u.ifn.entao);no_free(n->u.ifn.senao);break;
case N_WHILE: no_free(n->u.whilen.condicao);no_free(n->u.whilen.corpo);break;
case N_FOR: no_free(n->u.forn.inicio);no_free(n->u.forn.condicao);no_free(n->u.forn.passo);no_free(n->u.forn.corpo);break;
case N_RETURN: no_free(n->u.ret.expressao);break;
case N_BREAK: case N_CONTINUE: break;
case N_PRINT: no_free(n->u.printn.argumento);break;
case N_READ: no_free(n->u.readn.alvo);break;
case N_EXPRSTMT: no_free(n->u.exprstmt.expressao);break;
case N_ATRIB: no_free(n->u.atrib.alvo);no_free(n->u.atrib.valor);break;
case N_BINARIA: free(n->u.bin.operador);no_free(n->u.bin.esquerda);no_free(n->u.bin.direita);break;
case N_UNARIA: free(n->u.un.operador);no_free(n->u.un.operando);break;
case N_CHAMADA: no_free(n->u.chamada.funcao);freevec(&n->u.chamada.argumentos);break;
case N_INDICE: no_free(n->u.indice.vetor);no_free(n->u.indice.indice);break;
case N_IDENT: free(n->u.ident.nome);break;
case N_LITERAL: free(n->u.literal.lexema);free(n->u.literal.valor_str);break;
}}
static void printn(const No*n){if(!n){printf("NULL");return;}switch(n->kind){
case N_PROGRAMA: printf("Program(");for(size_t i=0;i<n->u.programa.declaracoes.n;i++){if(i)printf(", ");printn(n->u.programa.declaracoes.v[i]);}printf(")");break;
case N_FUNCAO: printf("Function(%s %s(",n->u.funcao.tipo_retorno,n->u.funcao.nome);for(size_t i=0;i<n->u.funcao.parametros.n;i++){if(i)printf(",");printn(n->u.funcao.parametros.v[i]);}printf(") ");printn(n->u.funcao.corpo);printf(")");break;
case N_PARAMETRO: printf("%s %s%s",n->u.parametro.tipo_base,n->u.parametro.nome,n->u.parametro.eh_vetor?"[]":"");break;
case N_DECLVAR: printf("VarDecl(%s %s",n->u.declvar.tipo_base,n->u.declvar.nome);if(n->u.declvar.tamanho){printf(" size=");printn(n->u.declvar.tamanho);}if(n->u.declvar.inicializador){printf("=");printn(n->u.declvar.inicializador);}printf(")");break;
case N_LISTADECL:for(size_t i=0;i<n->u.listadecl.declaracoes.n;i++){if(i)printf(", ");printn(n->u.listadecl.declaracoes.v[i]);}break;
case N_BLOCO: printf("Block(");for(size_t i=0;i<n->u.bloco.comandos.n;i++){if(i)printf(", ");printn(n->u.bloco.comandos.v[i]);}printf(")");break;
case N_IF: printf("If(");printn(n->u.ifn.condicao);printf(",");printn(n->u.ifn.entao);printf(",");printn(n->u.ifn.senao);printf(")");break;
case N_WHILE: printf("While(");printn(n->u.whilen.condicao);printf(",");printn(n->u.whilen.corpo);printf(")");break;
case N_FOR: printf("For(");printn(n->u.forn.inicio);printf(",");printn(n->u.forn.condicao);printf(",");printn(n->u.forn.passo);printf(",");printn(n->u.forn.corpo);printf(")");break;
case N_RETURN: printf("Return(");printn(n->u.ret.expressao);printf(")");break;
case N_BREAK: printf("Break");break;case N_CONTINUE:printf("Continue");break;
case N_PRINT: printf("Print(");printn(n->u.printn.argumento);printf(")");break;case N_READ:printf("Read(");printn(n->u.readn.alvo);printf(")");break;
case N_EXPRSTMT:printf("ExprStmt(");printn(n->u.exprstmt.expressao);printf(")");break;
case N_ATRIB:printf("Assign(");printn(n->u.atrib.alvo);printf(",");printn(n->u.atrib.valor);printf(")");break;
case N_BINARIA:printf("Binary(%s,",n->u.bin.operador);printn(n->u.bin.esquerda);printf(",");printn(n->u.bin.direita);printf(")");break;
case N_UNARIA:printf("Unary(%s,",n->u.un.operador);printn(n->u.un.operando);printf(")");break;
case N_CHAMADA:printf("Call(");printn(n->u.chamada.funcao);for(size_t i=0;i<n->u.chamada.argumentos.n;i++){printf(",");printn(n->u.chamada.argumentos.v[i]);}printf(")");break;
case N_INDICE:printf("Index(");printn(n->u.indice.vetor);printf(",");printn(n->u.indice.indice);printf(")");break;
case N_IDENT:printf("Id(%s)",n->u.ident.nome);break;
case N_LITERAL:{const char*t=n->u.literal.tipo_literal==L_INT?"int":n->u.literal.tipo_literal==L_REAL?"real":n->u.literal.tipo_literal==L_BOOL?"bool":n->u.literal.tipo_literal==L_CHAR?"char":"string";printf("Lit(%s,%s)",t,n->u.literal.lexema);break;}
}}
void ast_imprimir(const No*n){printn(n);}
