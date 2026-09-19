#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define MAX_TOKENS 10000
#define MAX_LEXEME 256

typedef struct {
    char token[32];
    char lexeme[MAX_LEXEME];
    int line;
    int column;
} Token;

static Token tokens[MAX_TOKENS];
static int total_tokens = 0;
static int posicao = 0;
static int erro_lexico = 0;

/* Utilidades de String Dinâmica */

static char *str_dup(const char *s) {
    char *p = malloc(strlen(s) + 1);
    if (p) strcpy(p, s);
    return p;
}

static char *str_concat2(const char *a, const char *b) {
    size_t la = strlen(a), lb = strlen(b);
    char *p = malloc(la + lb + 1);
    if (p) {
        memcpy(p, a, la);
        memcpy(p + la, b, lb);
        p[la + lb] = '\0';
    }
    return p;
}

static char *str_concat3(const char *a, const char *b, const char *c) {
    char *tmp = str_concat2(a, b);
    char *res = str_concat2(tmp, c);
    free(tmp);
    return res;
}

static char *str_concat_n(int n, ...) {
    va_list args;
    size_t total = 0;
    va_start(args, n);
    for (int i = 0; i < n; i++) {
        total += strlen(va_arg(args, const char *));
    }
    va_end(args);

    char *res = malloc(total + 1);
    if (!res) return NULL;

    size_t pos = 0;
    va_start(args, n);
    for (int i = 0; i < n; i++) {
        const char *s = va_arg(args, const char *);
        size_t l = strlen(s);
        memcpy(res + pos, s, l);
        pos += l;
    }
    va_end(args);
    res[pos] = '\0';
    return res;
}

/* Leitura de Tokens via scanner externo (equivalente ao subprocess do Python) */

static void extrair_campo_json(const char *linha, const char *campo,
                               char *destino, int tam) {
    char padrao[64];
    snprintf(padrao, sizeof(padrao), "\"%s\"", campo);

    const char *p = strstr(linha, padrao);
    if (!p) { destino[0] = '\0'; return; }

    p = strchr(p, ':');
    if (!p) { destino[0] = '\0'; return; }
    p++;

    while (*p == ' ' || *p == '\t') p++;

    if (*p == '"') {
        p++;
        int i = 0;
        while (*p && *p != '"' && i < tam - 1) {
            /* Desfaz escapes básicos do JSON */
            if (*p == '\\' && *(p + 1)) {
                p++;
                switch (*p) {
                    case 'n': destino[i++] = '\n'; break;
                    case 't': destino[i++] = '\t'; break;
                    case 'r': destino[i++] = '\r'; break;
                    case '"': destino[i++] = '"';  break;
                    case '\\': destino[i++] = '\\'; break;
                    default: destino[i++] = *p; break;
                }
                p++;
            } else {
                destino[i++] = *p++;
            }
        }
        destino[i] = '\0';
    } else {
        int i = 0;
        while (*p && *p != ',' && *p != '}' && i < tam - 1)
            destino[i++] = *p++;
        destino[i] = '\0';
    }
}

static int linha_so_espaco(const char *linha) {
    for (const char *q = linha; *q; q++) {
        if (*q != ' ' && *q != '\t' && *q != '\n' && *q != '\r')
            return 0;
    }
    return 1;
}

static int obter_tokens(const char *nome_arquivo) {
    char comando[1024];
    snprintf(comando, sizeof(comando), "./scanner %s", nome_arquivo);

    FILE *fp = popen(comando, "r");
    if (!fp) {
        fprintf(stderr, "erro: nao foi possivel executar o scanner\n");
        return 0;
    }

    total_tokens = 0;
    erro_lexico = 0;
    char linha[4096];

    while (fgets(linha, sizeof(linha), fp) && total_tokens < MAX_TOKENS) {
        if (linha_so_espaco(linha)) continue;

        /* Detecta linha de erro léxico: {"error":"...","lexeme":...} */
        if (strstr(linha, "\"error\"") != NULL) {
            erro_lexico = 1;
            break;
        }

        Token *t = &tokens[total_tokens];

        extrair_campo_json(linha, "token",  t->token,  sizeof(t->token));
        extrair_campo_json(linha, "lexeme", t->lexeme, sizeof(t->lexeme));

        char num[32];
        extrair_campo_json(linha, "line",   num, sizeof(num));
        t->line = atoi(num);

        extrair_campo_json(linha, "column", num, sizeof(num));
        t->column = atoi(num);

        total_tokens++;
    }

    pclose(fp);
    posicao = 0;
    return total_tokens;
}

/* Erro Sintático */

typedef struct {
    char mensagem[512];
} ErroSintatico;

static ErroSintatico erro_atual;
static int houve_erro = 0;

static void erro_sintatico(const char *mensagem) {
    Token *t = &tokens[posicao < total_tokens ? posicao : total_tokens - 1];
    snprintf(erro_atual.mensagem, sizeof(erro_atual.mensagem),
             "%s, encontrado %s ('%s') na linha %d, coluna %d",
             mensagem, t->token, t->lexeme, t->line, t->column);
    houve_erro = 1;
}

/* Utilidades de Navegação */

static Token *token_atual(void) {
    if (posicao < total_tokens)
        return &tokens[posicao];
    return &tokens[total_tokens - 1];
}

static const char *tipo_atual(void) {
    return token_atual()->token;
}

static int checar_varios(int n, ...) {
    va_list args;
    const char *tipo = tipo_atual();
    va_start(args, n);
    for (int i = 0; i < n; i++) {
        const char *esperado = va_arg(args, const char *);
        if (strcmp(tipo, esperado) == 0) {
            va_end(args);
            return 1;
        }
    }
    va_end(args);
    return 0;
}

#define NUM_ARGS(...) NUM_ARGS_IMPL(__VA_ARGS__, 10,9,8,7,6,5,4,3,2,1,0)
#define NUM_ARGS_IMPL(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,N,...) N
#define checar(...) checar_varios(NUM_ARGS(__VA_ARGS__), __VA_ARGS__)

static Token *avancar(void) {
    Token *t = token_atual();
    if (posicao < total_tokens - 1)
        posicao++;
    return t;
}

static Token *consumir(const char *tipo_esperado) {
    if (strcmp(tipo_atual(), tipo_esperado) != 0) {
        char msg[128];
        snprintf(msg, sizeof(msg), "esperado %s", tipo_esperado);
        erro_sintatico(msg);
        return token_atual();
    }
    return avancar();
}

static int eh_tipo(void) {
    const char *t = tipo_atual();
    return strcmp(t, "INT") == 0 || strcmp(t, "FLOAT") == 0 ||
           strcmp(t, "BOOL") == 0 || strcmp(t, "CHAR") == 0 ||
           strcmp(t, "VOID") == 0;
}

/* Protótipos */

static char *parse_programa(void);
static char *parse_declaracao_global(void);
static char *parse_funcao(const char *tipo, const char *nome);
static char *parse_parametro(void);
static char *parse_var_decl_resto(const char *tipo, const char *nome);
static char *parse_bloco(void);
static char *parse_comando(void);
static char *parse_if(void);
static char *parse_while(void);
static char *parse_return(void);
static char *parse_expr_stmt(void);
static char *parse_expressao(void);
static char *parse_atribuicao(void);
static char *parse_or(void);
static char *parse_and(void);
static char *parse_igualdade(void);
static char *parse_relacional(void);
static char *parse_aditiva(void);
static char *parse_multiplicativa(void);
static char *parse_unaria(void);
static char *parse_pos_fixo(void);
static char *parse_primario(void);

/* Programa / Declarações Globais */

static char *parse_programa(void) {
    char **declaracoes = NULL;
    int count = 0, cap = 16;
    declaracoes = malloc(cap * sizeof(char *));

    while (strcmp(tipo_atual(), "EOF") != 0 && !houve_erro) {
        if (count >= cap) {
            cap *= 2;
            declaracoes = realloc(declaracoes, cap * sizeof(char *));
        }
        declaracoes[count++] = parse_declaracao_global();
    }

    size_t total = 9;
    for (int i = 0; i < count; i++) {
        total += strlen(declaracoes[i]);
        if (i > 0) total += 2;
    }

    char *resultado = malloc(total);
    strcpy(resultado, "Program(");
    for (int i = 0; i < count; i++) {
        if (i > 0) strcat(resultado, ", ");
        strcat(resultado, declaracoes[i]);
        free(declaracoes[i]);
    }
    strcat(resultado, ")");
    free(declaracoes);
    return resultado;
}

static char *parse_declaracao_global(void) {
    if (eh_tipo()) {
        Token *tipo_tok = avancar();
        char tipo[64];
        strncpy(tipo, tipo_tok->lexeme, sizeof(tipo) - 1);
        tipo[sizeof(tipo) - 1] = '\0';

        Token *nome_tok = consumir("IDENT");
        char nome[64];
        strncpy(nome, nome_tok->lexeme, sizeof(nome) - 1);
        nome[sizeof(nome) - 1] = '\0';

        if (strcmp(tipo_atual(), "LPAREN") == 0) {
            return parse_funcao(tipo, nome);
        }
        return parse_var_decl_resto(tipo, nome);
    }

    return parse_comando();
}

static char *parse_funcao(const char *tipo, const char *nome) {
    consumir("LPAREN");

    char **params = NULL;
    int count = 0, cap = 8;
    params = malloc(cap * sizeof(char *));

    if (strcmp(tipo_atual(), "RPAREN") != 0) {
        params[count++] = parse_parametro();
        while (strcmp(tipo_atual(), "COMMA") == 0) {
            avancar();
            if (count >= cap) {
                cap *= 2;
                params = realloc(params, cap * sizeof(char *));
            }
            params[count++] = parse_parametro();
        }
    }

    consumir("RPAREN");

    char *bloco = parse_bloco();

    size_t total_params = 0;
    for (int i = 0; i < count; i++) {
        total_params += strlen(params[i]);
        if (i > 0) total_params += 1;
    }

    char *params_str = malloc(total_params + 1);
    params_str[0] = '\0';
    for (int i = 0; i < count; i++) {
        if (i > 0) strcat(params_str, ",");
        strcat(params_str, params[i]);
        free(params[i]);
    }
    free(params);

    char *resultado = str_concat_n(6,
        "Function(", tipo, " ", nome, "(", params_str);
    char *resultado2 = str_concat_n(4, resultado, ") ", bloco, ")");
    free(resultado);
    free(params_str);
    free(bloco);
    return resultado2;
}

static char *parse_parametro(void) {
    if (!eh_tipo()) {
        erro_sintatico("esperado tipo de parâmetro");
        return str_dup("ERRO");
    }

    Token *tipo_tok = avancar();
    Token *nome_tok = consumir("IDENT");

    return str_concat_n(3, tipo_tok->lexeme, " ", nome_tok->lexeme);
}

static char *parse_var_decl_resto(const char *tipo, const char *nome) {
    char *tamanho = NULL;
    char *inicializador = NULL;

    if (strcmp(tipo_atual(), "LBRACKET") == 0) {
        avancar();
        tamanho = parse_expressao();
        consumir("RBRACKET");
    }

    if (strcmp(tipo_atual(), "ASSIGN") == 0) {
        avancar();
        inicializador = parse_expressao();
    }

    consumir("SEMICOLON");

    char *texto = str_concat_n(3, tipo, " ", nome);

    if (tamanho) {
        char *tmp = str_concat_n(3, texto, " size=", tamanho);
        free(texto);
        texto = tmp;
        free(tamanho);
    }

    if (inicializador) {
        char *tmp = str_concat_n(3, texto, "=", inicializador);
        free(texto);
        texto = tmp;
        free(inicializador);
    }

    char *resultado = str_concat_n(3, "VarDecl(", texto, ")");
    free(texto);
    return resultado;
}

/* Comandos */

static char *parse_bloco(void) {
    consumir("LBRACE");

    char **comandos = NULL;
    int count = 0, cap = 16;
    comandos = malloc(cap * sizeof(char *));

    while (strcmp(tipo_atual(), "RBRACE") != 0 && !houve_erro) {
        if (count >= cap) {
            cap *= 2;
            comandos = realloc(comandos, cap * sizeof(char *));
        }
        comandos[count++] = parse_comando();
    }

    consumir("RBRACE");

    size_t total = 8;
    for (int i = 0; i < count; i++) {
        total += strlen(comandos[i]);
        if (i > 0) total += 2;
    }

    char *resultado = malloc(total);
    strcpy(resultado, "Block(");
    for (int i = 0; i < count; i++) {
        if (i > 0) strcat(resultado, ", ");
        strcat(resultado, comandos[i]);
        free(comandos[i]);
    }
    strcat(resultado, ")");
    free(comandos);
    return resultado;
}

static char *parse_comando(void) {
    if (eh_tipo()) {
        Token *tipo_tok = avancar();
        char tipo[64];
        strncpy(tipo, tipo_tok->lexeme, sizeof(tipo) - 1);
        tipo[sizeof(tipo) - 1] = '\0';

        Token *nome_tok = consumir("IDENT");
        char nome[64];
        strncpy(nome, nome_tok->lexeme, sizeof(nome) - 1);
        nome[sizeof(nome) - 1] = '\0';

        return parse_var_decl_resto(tipo, nome);
    }

    if (strcmp(tipo_atual(), "IF") == 0) return parse_if();
    if (strcmp(tipo_atual(), "WHILE") == 0) return parse_while();
    if (strcmp(tipo_atual(), "RETURN") == 0) return parse_return();
    if (strcmp(tipo_atual(), "LBRACE") == 0) return parse_bloco();

    return parse_expr_stmt();
}

static char *parse_if(void) {
    consumir("IF");
    consumir("LPAREN");
    char *condicao = parse_expressao();
    consumir("RPAREN");

    char *entao = parse_comando();

    char *senao = NULL;
    if (strcmp(tipo_atual(), "ELSE") == 0) {
        avancar();
        senao = parse_comando();
    }

    char *resultado;
    if (senao) {
        resultado = str_concat_n(7,
            "If(", condicao, ",", entao, ",", senao, ")");
        free(senao);
    } else {
        resultado = str_concat_n(5,
            "If(", condicao, ",", entao, ",NULL)");
    }

    free(condicao);
    free(entao);
    return resultado;
}

static char *parse_while(void) {
    consumir("WHILE");
    consumir("LPAREN");
    char *condicao = parse_expressao();
    consumir("RPAREN");

    char *corpo = parse_comando();

    char *resultado = str_concat_n(5,
        "While(", condicao, ",", corpo, ")");
    free(condicao);
    free(corpo);
    return resultado;
}

static char *parse_return(void) {
    consumir("RETURN");

    if (strcmp(tipo_atual(), "SEMICOLON") == 0) {
        avancar();
        return str_dup("Return(NULL)");
    }

    char *expressao = parse_expressao();
    consumir("SEMICOLON");

    char *resultado = str_concat_n(3, "Return(", expressao, ")");
    free(expressao);
    return resultado;
}

static char *parse_expr_stmt(void) {
    char *expressao = parse_expressao();
    consumir("SEMICOLON");

    char *resultado = str_concat_n(3, "ExprStmt(", expressao, ")");
    free(expressao);
    return resultado;
}

/* Expressões (Precedência crescente) */

static char *parse_expressao(void) {
    return parse_atribuicao();
}

static char *parse_atribuicao(void) {
    char *esquerda = parse_or();

    if (strcmp(tipo_atual(), "ASSIGN") == 0) {
        if (!(strncmp(esquerda, "Id(", 3) == 0 ||
              strncmp(esquerda, "Index(", 6) == 0)) {
            erro_sintatico("alvo de atribuição inválido");
            free(esquerda);
            return str_dup("ERRO");
        }

        avancar();
        char *direita = parse_atribuicao();
        char *resultado = str_concat_n(5,
            "Assign(", esquerda, ",", direita, ")");
        free(esquerda);
        free(direita);
        return resultado;
    }

    return esquerda;
}

static char *parse_or(void) {
    char *esquerda = parse_and();
    while (strcmp(tipo_atual(), "OR") == 0) {
        Token *op = avancar();
        char *direita = parse_and();
        char *resultado = str_concat_n(7,
            "Binary(", op->lexeme, ",", esquerda, ",", direita, ")");
        free(esquerda);
        free(direita);
        esquerda = resultado;
    }
    return esquerda;
}

static char *parse_and(void) {
    char *esquerda = parse_igualdade();
    while (strcmp(tipo_atual(), "AND") == 0) {
        Token *op = avancar();
        char *direita = parse_igualdade();
        char *resultado = str_concat_n(7,
            "Binary(", op->lexeme, ",", esquerda, ",", direita, ")");
        free(esquerda);
        free(direita);
        esquerda = resultado;
    }
    return esquerda;
}

static char *parse_igualdade(void) {
    char *esquerda = parse_relacional();
    while (strcmp(tipo_atual(), "EQ") == 0 ||
           strcmp(tipo_atual(), "NE") == 0) {
        Token *op = avancar();
        char *direita = parse_relacional();
        char *resultado = str_concat_n(7,
            "Binary(", op->lexeme, ",", esquerda, ",", direita, ")");
        free(esquerda);
        free(direita);
        esquerda = resultado;
    }
    return esquerda;
}

static char *parse_relacional(void) {
    char *esquerda = parse_aditiva();
    while (strcmp(tipo_atual(), "LT") == 0 ||
           strcmp(tipo_atual(), "GT") == 0 ||
           strcmp(tipo_atual(), "LE") == 0 ||
           strcmp(tipo_atual(), "GE") == 0) {
        Token *op = avancar();
        char *direita = parse_aditiva();
        char *resultado = str_concat_n(7,
            "Binary(", op->lexeme, ",", esquerda, ",", direita, ")");
        free(esquerda);
        free(direita);
        esquerda = resultado;
    }
    return esquerda;
}

static char *parse_aditiva(void) {
    char *esquerda = parse_multiplicativa();
    while (strcmp(tipo_atual(), "PLUS") == 0 ||
           strcmp(tipo_atual(), "MINUS") == 0) {
        Token *op = avancar();
        char *direita = parse_multiplicativa();
        char *resultado = str_concat_n(7,
            "Binary(", op->lexeme, ",", esquerda, ",", direita, ")");
        free(esquerda);
        free(direita);
        esquerda = resultado;
    }
    return esquerda;
}

static char *parse_multiplicativa(void) {
    char *esquerda = parse_unaria();
    while (strcmp(tipo_atual(), "STAR") == 0 ||
           strcmp(tipo_atual(), "SLASH") == 0 ||
           strcmp(tipo_atual(), "PERCENT") == 0) {
        Token *op = avancar();
        char *direita = parse_unaria();
        char *resultado = str_concat_n(7,
            "Binary(", op->lexeme, ",", esquerda, ",", direita, ")");
        free(esquerda);
        free(direita);
        esquerda = resultado;
    }
    return esquerda;
}

static char *parse_unaria(void) {
    if (strcmp(tipo_atual(), "MINUS") == 0 ||
        strcmp(tipo_atual(), "NOT") == 0) {
        Token *op = avancar();
        char *operando = parse_unaria();
        char *resultado = str_concat_n(5,
            "Unary(", op->lexeme, ",", operando, ")");
        free(operando);
        return resultado;
    }
    return parse_pos_fixo();
}

static char *parse_pos_fixo(void) {
    char *expressao = parse_primario();

    while (1) {
        if (strcmp(tipo_atual(), "LPAREN") == 0) {
            avancar();
            char **argumentos = NULL;
            int count = 0, cap = 8;
            argumentos = malloc(cap * sizeof(char *));

            if (strcmp(tipo_atual(), "RPAREN") != 0) {
                argumentos[count++] = parse_expressao();
                while (strcmp(tipo_atual(), "COMMA") == 0) {
                    avancar();
                    if (count >= cap) {
                        cap *= 2;
                        argumentos = realloc(argumentos, cap * sizeof(char *));
                    }
                    argumentos[count++] = parse_expressao();
                }
            }
            consumir("RPAREN");

            size_t total_args = 0;
            for (int i = 0; i < count; i++) {
                total_args += strlen(argumentos[i]);
                if (i > 0) total_args += 1;
            }

            char *args_str = malloc(total_args + 1);
            args_str[0] = '\0';
            for (int i = 0; i < count; i++) {
                if (i > 0) strcat(args_str, ",");
                strcat(args_str, argumentos[i]);
                free(argumentos[i]);
            }
            free(argumentos);

            char *resultado;
            if (count > 0) {
                resultado = str_concat_n(4,
                    "Call(", expressao, ",", args_str);
                char *tmp = str_concat_n(3, resultado, ")", "");
                free(resultado);
                resultado = tmp;
            } else {
                resultado = str_concat_n(3, "Call(", expressao, ")");
            }

            free(args_str);
            free(expressao);
            expressao = resultado;
        }
        else if (strcmp(tipo_atual(), "LBRACKET") == 0) {
            avancar();
            char *indice = parse_expressao();
            consumir("RBRACKET");
            char *resultado = str_concat_n(5,
                "Index(", expressao, ",", indice, ")");
            free(indice);
            free(expressao);
            expressao = resultado;
        }
        else {
            break;
        }
    }

    return expressao;
}

static char *parse_primario(void) {
    Token *token = token_atual();
    const char *tipo = token->token;

    if (strcmp(tipo, "IDENT") == 0) {
        avancar();
        return str_concat_n(3, "Id(", token->lexeme, ")");
    }

    if (strcmp(tipo, "INT_LIT") == 0) {
        avancar();
        return str_concat_n(3, "Lit(int,", token->lexeme, ")");
    }

    if (strcmp(tipo, "FLOAT_LIT") == 0) {
        avancar();
        return str_concat_n(3, "Lit(real,", token->lexeme, ")");
    }

    if (strcmp(tipo, "TRUE") == 0) {
        avancar();
        return str_dup("Lit(bool,true)");
    }

    if (strcmp(tipo, "FALSE") == 0) {
        avancar();
        return str_dup("Lit(bool,false)");
    }

    if (strcmp(tipo, "STRING_LIT") == 0) {
        avancar();
        return str_concat_n(3, "Lit(string,", token->lexeme, ")");
    }

    if (strcmp(tipo, "CHAR_LIT") == 0) {
        avancar();
        return str_concat_n(3, "Lit(char,", token->lexeme, ")");
    }

    if (strcmp(tipo, "LPAREN") == 0) {
        avancar();
        char *expressao = parse_expressao();
        consumir("RPAREN");
        return expressao;
    }

    erro_sintatico("esperado identificador, literal ou '('");
    return str_dup("ERRO");
}

/* Main */

#define MENSAGEM_REJEICAO "NÃO HÁ AST: o parser deve rejeitar a entrada."

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <arquivo.c>\n", argv[0]);
        return 2;
    }

    const char *nome_arquivo = argv[1];

    if (obter_tokens(nome_arquivo) <= 0) {
        printf("%s\n", MENSAGEM_REJEICAO);
        return 1;
    }

    if (erro_lexico) {
        printf("%s\n", MENSAGEM_REJEICAO);
        return 1;
    }

    if (total_tokens == 0 ||
        strcmp(tokens[total_tokens - 1].token, "EOF") != 0) {
        printf("%s\n", MENSAGEM_REJEICAO);
        return 1;
    }

    char *ast = parse_programa();

    if (houve_erro) {
        printf("%s\n", MENSAGEM_REJEICAO);
        free(ast);
        return 1;
    }

    printf("%s\n", ast);
    free(ast);
    return 0;
}