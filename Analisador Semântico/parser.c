/*
 * parser.c — Parser descendente recursivo.
 */
#include "util.h"
#include "scanner.h"
#include "parser.h"

#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

static int posicao;
static Token *ultimo;           /* último token consumido */
static jmp_buf salto_erro;
char *msg_erro_sintatico;

static Token *token_atual(void)
{
    return &tokens[posicao < n_tokens ? posicao : n_tokens - 1];
}

static int checar(TipoToken tipo)
{
    return token_atual()->tipo == tipo;
}

static int checar_tipo(void)
{
    TipoToken t = token_atual()->tipo;
    return t == T_INT || t == T_FLOAT || t == T_BOOL || t == T_CHAR || t == T_VOID;
}

static Token *avancar(void)
{
    Token *token = token_atual();
    if (posicao < n_tokens - 1)
        posicao++;
    ultimo = token;
    return token;
}

static void erro_sintatico(const char *mensagem) __attribute__((noreturn));

static void erro_sintatico(const char *mensagem)
{
    Token *t = token_atual();
    msg_erro_sintatico = fmt("%s, encontrado %s ('%s') na linha %d, coluna %d",
                             mensagem, NOME_TOKEN[t->tipo], t->lexema,
                             t->linha, t->coluna);
    longjmp(salto_erro, 1);
}

static Token *consumir(TipoToken esperado)
{
    if (!checar(esperado))
        erro_sintatico(fmt("esperado %s", NOME_TOKEN[esperado]));
    return avancar();
}

/* Define o trecho do fonte de uma expressão: começa em um token ou em
 * outro nó e termina logo após o último token consumido. */
static No *marcar_token(No *no, Token *origem)
{
    no->linha = origem->linha;
    no->coluna = origem->coluna;
    no->ini = origem->ini;
    no->fim = ultimo->fim;
    no->tem_trecho = 1;
    return no;
}

static No *marcar_no(No *no, No *origem)
{
    no->linha = origem->linha;
    no->coluna = origem->coluna;
    no->ini = origem->ini;
    no->fim = ultimo->fim;
    no->tem_trecho = 1;
    return no;
}

static No *parse_expressao(void);
static No *parse_comando(void);
static No *parse_bloco(void);

/* Programa / declarações */

static No *parse_declarador(const char *tipo, Token *tok_nome)
{
    No *no = novo_no(N_DECLVAR, tok_nome->linha, tok_nome->coluna);
    no->tipo_base = tipo;
    no->nome = tok_nome->lexema;

    if (checar(T_LBRACKET)) {
        avancar();
        no->tamanho = parse_expressao();
        consumir(T_RBRACKET);
    }
    if (checar(T_ASSIGN)) {
        avancar();
        no->inicializador = parse_expressao();
    }
    return no;
}

static No *parse_var_decl_resto(Token *tok_tipo, Token *tok_nome)
{
    No *lista = novo_no(N_LISTADECL, tok_tipo->linha, tok_tipo->coluna);
    lista_adicionar(&lista->lista, parse_declarador(tok_tipo->lexema, tok_nome));

    while (checar(T_COMMA)) {
        Token *proximo;
        avancar();
        proximo = consumir(T_IDENT);
        lista_adicionar(&lista->lista, parse_declarador(tok_tipo->lexema, proximo));
    }
    consumir(T_SEMICOLON);

    if (lista->lista.n == 1)
        return lista->lista.itens[0];
    return lista;
}

static No *parse_parametro(void)
{
    const char *tipo;
    Token *tok_nome;
    No *no;

    if (!checar_tipo())
        erro_sintatico("esperado tipo de parâmetro");

    tipo = avancar()->lexema;
    tok_nome = consumir(T_IDENT);

    no = novo_no(N_PARAMETRO, tok_nome->linha, tok_nome->coluna);
    no->tipo_base = tipo;
    no->nome = tok_nome->lexema;

    if (checar(T_LBRACKET)) {
        avancar();
        consumir(T_RBRACKET);
        no->eh_vetor = 1;
    }
    return no;
}

static No *parse_funcao(Token *tok_tipo, Token *tok_nome)
{
    /* A posição do nó é o tipo de retorno. */
    No *no = novo_no(N_FUNCAO, tok_tipo->linha, tok_tipo->coluna);
    no->tipo_base = tok_tipo->lexema;
    no->nome = tok_nome->lexema;
    no->linha_nome = tok_nome->linha;
    no->coluna_nome = tok_nome->coluna;

    consumir(T_LPAREN);
    if (!checar(T_RPAREN)) {
        lista_adicionar(&no->lista, parse_parametro());
        while (checar(T_COMMA)) {
            avancar();
            lista_adicionar(&no->lista, parse_parametro());
        }
    }
    consumir(T_RPAREN);

    /* A gramática não admite protótipo: o corpo é obrigatório. */
    no->corpo = parse_bloco();
    return no;
}

static No *parse_declaracao_global(void)
{
    if (checar_tipo()) {
        Token *tok_tipo = avancar();
        Token *tok_nome = consumir(T_IDENT);

        if (checar(T_LPAREN))
            return parse_funcao(tok_tipo, tok_nome);
        return parse_var_decl_resto(tok_tipo, tok_nome);
    }
    return parse_comando();
}

static No *parse_programa(void)
{
    Token *inicio = token_atual();
    No *no = novo_no(N_PROGRAMA, inicio->linha, inicio->coluna);

    while (!checar(T_EOF))
        lista_adicionar(&no->lista, parse_declaracao_global());
    return no;
}

/* Comandos */

static No *parse_bloco(void)
{
    Token *tok = consumir(T_LBRACE);
    No *no = novo_no(N_BLOCO, tok->linha, tok->coluna);

    while (!checar(T_RBRACE)) {
        if (checar(T_EOF))
            erro_sintatico("esperado RBRACE");
        lista_adicionar(&no->lista, parse_comando());
    }
    consumir(T_RBRACE);
    return no;
}

static No *parse_if(void)
{
    Token *tok = consumir(T_IF);
    No *no = novo_no(N_IF, tok->linha, tok->coluna);

    consumir(T_LPAREN);
    no->condicao = parse_expressao();
    consumir(T_RPAREN);

    no->entao = parse_comando();
    if (checar(T_ELSE)) {
        avancar();
        no->senao = parse_comando();
    }
    return no;
}

static No *parse_while(void)
{
    Token *tok = consumir(T_WHILE);
    No *no = novo_no(N_WHILE, tok->linha, tok->coluna);

    consumir(T_LPAREN);
    no->condicao = parse_expressao();
    consumir(T_RPAREN);
    no->corpo = parse_comando();
    return no;
}

static No *parse_for(void)
{
    Token *tok = consumir(T_FOR);
    No *no = novo_no(N_FOR, tok->linha, tok->coluna);

    consumir(T_LPAREN);
    no->inicio = checar(T_SEMICOLON) ? NULL : parse_expressao();
    consumir(T_SEMICOLON);
    no->condicao = checar(T_SEMICOLON) ? NULL : parse_expressao();
    consumir(T_SEMICOLON);
    no->passo = checar(T_RPAREN) ? NULL : parse_expressao();
    consumir(T_RPAREN);
    no->corpo = parse_comando();
    return no;
}

static No *parse_return(void)
{
    Token *tok = consumir(T_RETURN);
    No *no = novo_no(N_RETURN, tok->linha, tok->coluna);

    if (checar(T_SEMICOLON)) {
        avancar();
        return no;
    }
    no->expressao = parse_expressao();
    consumir(T_SEMICOLON);
    return no;
}

static No *parse_print(void)
{
    Token *tok = consumir(T_PRINT);
    No *no = novo_no(N_PRINT, tok->linha, tok->coluna);

    consumir(T_LPAREN);
    no->argumento = parse_expressao();
    consumir(T_RPAREN);
    consumir(T_SEMICOLON);
    return no;
}

static No *parse_read(void)
{
    Token *tok = consumir(T_READ);
    No *no = novo_no(N_READ, tok->linha, tok->coluna);

    consumir(T_LPAREN);
    no->alvo = parse_expressao();   /* validado na semântica (SEM013) */
    consumir(T_RPAREN);
    consumir(T_SEMICOLON);
    return no;
}

static No *parse_expr_stmt(void)
{
    Token *tok = token_atual();
    No *no = novo_no(N_EXPRSTMT, tok->linha, tok->coluna);

    if (checar(T_SEMICOLON)) {
        avancar();
        return no;
    }
    no->expressao = parse_expressao();
    consumir(T_SEMICOLON);
    return no;
}

static No *parse_comando(void)
{
    Token *tok;

    if (checar_tipo()) {
        Token *tok_tipo = avancar();
        Token *tok_nome = consumir(T_IDENT);
        return parse_var_decl_resto(tok_tipo, tok_nome);
    }

    if (checar(T_IF))     return parse_if();
    if (checar(T_WHILE))  return parse_while();
    if (checar(T_FOR))    return parse_for();
    if (checar(T_RETURN)) return parse_return();
    if (checar(T_BREAK)) {
        tok = avancar();
        consumir(T_SEMICOLON);
        return novo_no(N_BREAK, tok->linha, tok->coluna);
    }
    if (checar(T_CONTINUE)) {
        tok = avancar();
        consumir(T_SEMICOLON);
        return novo_no(N_CONTINUE, tok->linha, tok->coluna);
    }
    if (checar(T_PRINT))  return parse_print();
    if (checar(T_READ))   return parse_read();
    if (checar(T_LBRACE)) return parse_bloco();

    return parse_expr_stmt();
}

/* Expressões (precedência crescente) */

static No *parse_primario(void)
{
    Token *token = token_atual();
    No *no;

    switch (token->tipo) {
    case T_IDENT:
        avancar();
        no = novo_no(N_IDENTIFICADOR, 0, 0);
        no->nome = token->lexema;
        return marcar_token(no, token);

    case T_INT_LIT: case T_FLOAT_LIT: case T_TRUE: case T_FALSE:
    case T_STRING_LIT: case T_CHAR_LIT:
        avancar();
        no = novo_no(N_LITERAL, 0, 0);
        no->lexema = token->lexema;
        switch (token->tipo) {
        case T_INT_LIT:
            no->tipo_literal = "int";
            no->valor_lit.ok = 1;
            no->valor_lit.inteiro = 1;
            no->valor_lit.i = strtoll(token->lexema, NULL, 10);
            break;
        case T_FLOAT_LIT:
            no->tipo_literal = "real";
            no->valor_lit.ok = 1;
            no->valor_lit.r = strtod(token->lexema, NULL);
            break;
        case T_STRING_LIT:
            no->tipo_literal = "string";
            break;
        case T_CHAR_LIT:
            no->tipo_literal = "char";
            break;
        default:   /* T_TRUE, T_FALSE */
            no->tipo_literal = "bool";
            break;
        }
        return marcar_token(no, token);

    case T_LPAREN:
        avancar();
        no = parse_expressao();
        consumir(T_RPAREN);
        /* O trecho passa a incluir os parênteses; assim o texto de
         * "(a + b) * c" é recortado corretamente do fonte. */
        return marcar_token(no, token);

    default:
        erro_sintatico("esperado identificador, literal ou '('");
    }
}

static No *parse_pos_fixo(void)
{
    No *expressao = parse_primario();

    for (;;) {
        if (checar(T_LPAREN)) {
            No *chamada = novo_no(N_CHAMADA, 0, 0);
            chamada->funcao = expressao;
            avancar();
            if (!checar(T_RPAREN)) {
                lista_adicionar(&chamada->lista, parse_expressao());
                while (checar(T_COMMA)) {
                    avancar();
                    lista_adicionar(&chamada->lista, parse_expressao());
                }
            }
            consumir(T_RPAREN);
            expressao = marcar_no(chamada, expressao);
        } else if (checar(T_LBRACKET)) {
            No *indice = novo_no(N_INDICE, 0, 0);
            indice->vetor = expressao;
            avancar();
            indice->indice = parse_expressao();
            consumir(T_RBRACKET);
            expressao = marcar_no(indice, expressao);
        } else {
            break;
        }
    }
    return expressao;
}

static No *parse_unaria(void)
{
    if (checar(T_MINUS) || checar(T_NOT)) {
        Token *tok = avancar();
        No *no = novo_no(N_UNARIA, 0, 0);
        no->operador = tok->lexema;
        no->operando = parse_unaria();
        return marcar_token(no, tok);
    }
    return parse_pos_fixo();
}

/* Laço genérico para operadores binários associativos à esquerda. */
static No *parse_binaria(No *(*proximo)(void), const TipoToken *ops, int n_ops)
{
    No *esquerda = proximo();

    for (;;) {
        int i, achou = 0;
        Token *tok;
        No *no;

        for (i = 0; i < n_ops; i++)
            if (checar(ops[i]))
                achou = 1;
        if (!achou)
            break;

        tok = avancar();
        no = novo_no(N_BINARIA, 0, 0);
        no->operador = tok->lexema;
        no->esquerda = esquerda;
        no->direita = proximo();
        esquerda = marcar_no(no, esquerda);
    }
    return esquerda;
}

static No *parse_multiplicativa(void)
{
    static const TipoToken ops[] = { T_STAR, T_SLASH, T_PERCENT };
    return parse_binaria(parse_unaria, ops, 3);
}

static No *parse_aditiva(void)
{
    static const TipoToken ops[] = { T_PLUS, T_MINUS };
    return parse_binaria(parse_multiplicativa, ops, 2);
}

static No *parse_relacional(void)
{
    static const TipoToken ops[] = { T_LT, T_GT, T_LE, T_GE };
    return parse_binaria(parse_aditiva, ops, 4);
}

static No *parse_igualdade(void)
{
    static const TipoToken ops[] = { T_EQ, T_NE };
    return parse_binaria(parse_relacional, ops, 2);
}

static No *parse_and(void)
{
    static const TipoToken ops[] = { T_AND };
    return parse_binaria(parse_igualdade, ops, 1);
}

static No *parse_or(void)
{
    static const TipoToken ops[] = { T_OR };
    return parse_binaria(parse_and, ops, 1);
}

static No *parse_atribuicao(void)
{
    No *esquerda = parse_or();

    if (checar(T_ASSIGN)) {
        No *no = novo_no(N_ATRIBUICAO, 0, 0);
        avancar();
        no->alvo = esquerda;
        no->valor = parse_atribuicao();   /* associativa à direita */
        return marcar_no(no, esquerda);
    }
    return esquerda;
}

static No *parse_expressao(void)
{
    return parse_atribuicao();
}

/* Ponto de entrada: devolve a árvore, ou NULL em caso de erro sintático.
 * O setjmp fica isolado nesta função para que nenhuma variável local seja
 * afetada pelo longjmp. */
No *analisar_sintaxe(void)
{
    posicao = 0;
    ultimo = NULL;
    if (setjmp(salto_erro))
        return NULL;
    return parse_programa();
}
