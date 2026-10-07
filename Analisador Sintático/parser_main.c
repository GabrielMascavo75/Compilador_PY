#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
static char*rd(const char*p){FILE*f=fopen(p,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long n=ftell(f);rewind(f);char*s=malloc(n+1);fread(s,1,n,f);s[n]=0;fclose(f);return s;}
int main(int ac,char**av){if(ac!=2){fprintf(stderr,"Uso: ./parser <arquivo.c>\n");return 2;}char*s=rd(av[1]);if(!s){fprintf(stderr,"Arquivo não encontrado: %s\n",av[1]);return 2;}Parser p;if(!parser_init(&p,s,1)){printf("NÃO HÁ AST: o parser deve rejeitar a entrada.\n");parser_free(&p);free(s);return 1;}No*ast=analisar_sintaxe(&p);if(!ast){printf("NÃO HÁ AST: o parser deve rejeitar a entrada.\n");parser_free(&p);free(s);return 1;}ast_imprimir(ast);putchar('\n');no_free(ast);parser_free(&p);free(s);return 0;}
