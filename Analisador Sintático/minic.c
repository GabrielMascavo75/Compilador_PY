#include "parser.h"
#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static char*readall(const char*p){FILE*f=fopen(p,"rb");if(!f)return NULL;fseek(f,0,SEEK_END);long n=ftell(f);fseek(f,0,SEEK_SET);char*s=malloc((size_t)n+1);if(!s){fclose(f);return NULL;}fread(s,1,(size_t)n,f);s[n]=0;fclose(f);return s;}
int main(int argc,char**argv){if(argc!=2){fprintf(stderr,"Uso: ./minic <arquivo.c>\n");return 1;}FILE*f=fopen(argv[1],"rb");if(!f){fprintf(stderr,"Arquivo não encontrado: %s\n",argv[1]);return 1;}fclose(f);char*src=readall(argv[1]);Parser p;if(!parser_init(&p,src,0)){printf("Erro léxico: %s\n",p.toks.erro_msg?p.toks.erro_msg:"tokens inválidos");parser_free(&p);free(src);return 2;}No*ast=analisar_sintaxe(&p);if(!ast){printf("Erro sintático: %s\n",p.erro?p.erro:"erro sintático");parser_free(&p);free(src);return 3;}Semantic*s=semantic_new(src);DiagnosticoVec*e=semantic_analisar(s,ast);for(size_t i=0;i<e->n;i++)printf("%s — linha %d, coluna %d: %s\n",e->v[i].codigo,e->v[i].linha,e->v[i].coluna,e->v[i].mensagem);printf("Análise semântica concluída: %zu %s; programa %s.\n",e->n,e->n==1?"erro":"erros",e->n?"rejeitado":"aceito");semantic_free(s);no_free(ast);parser_free(&p);free(src);return e->n?4:0;}
