#include "scanner.h"
#include <stdio.h>
#include <stdlib.h>
static char*rd(const char*p){FILE*f=fopen(p,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long n=ftell(f);rewind(f);char*s=malloc(n+1);fread(s,1,n,f);s[n]=0;fclose(f);return s;}
int main(int ac,char**av){if(ac!=2){fprintf(stderr,"Uso: ./scanner <arquivo.c>\n");return 1;}char*s=rd(av[1]);if(!s){perror(av[1]);return 1;}TokenVec v;if(!scanner_ler(s,&v)){fprintf(stderr,"Erro léxico: %s\n",v.erro_msg?v.erro_msg:"erro");tokens_free(&v);free(s);return 2;}for(size_t i=0;i<v.n;i++)printf("%s '%s' %d:%d\n",token_nome(v.v[i].tipo),v.v[i].lexema,v.v[i].linha,v.v[i].coluna);tokens_free(&v);free(s);return 0;}
