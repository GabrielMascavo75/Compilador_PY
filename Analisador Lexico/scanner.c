#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <ctype.h>

/* ============================================================
   ESPECIFICAÇÃO DOS TOKENS
   ============================================================ */

typedef struct {
    const char *name;
    const char *pattern;
} TokenSpec;

TokenSpec TOKEN_SPEC[] = {

    {"AUTO",       "auto"},
    {"BREAK",      "break"},
    {"CASE",       "case"},
    {"CHAR",       "char"},
    {"CONST",      "const"},
    {"CONTINUE",   "continue"},
    {"DEFAULT",    "default"},
    {"DO",         "do"},
    {"DOUBLE",     "double"},
    {"ELSE",       "else"},
    {"ENUM",       "enum"},
    {"EXTERN",     "extern"},
    {"FLOAT",      "float"},
    {"FOR",        "for"},
    {"GOTO",       "goto"},
    {"IF",         "if"},
    {"INT",        "int"},
    {"LONG",       "long"},
    {"REGISTER",   "register"},
    {"RETURN",     "return"},
    {"SHORT",      "short"},
    {"SIGNED",     "signed"},
    {"SIZEOF",     "sizeof"},
    {"STATIC",     "static"},
    {"STRUCT",     "struct"},
    {"SWITCH",     "switch"},
    {"TYPEDEF",    "typedef"},
    {"UNION",      "union"},
    {"UNSIGNED",   "unsigned"},
    {"VOID",       "void"},
    {"VOLATILE",   "volatile"},
    {"WHILE",      "while"},
    {"BOOL",       "bool"},
    {"TRUE",       "true"},
    {"FALSE",      "false"},
    {"PRINT",      "print"},

    {"COMMENT",       "//[^\n]*"},
    {"BLOCK_COMMENT", "/\\*[\\s\\S]*\\*/"},

    {"IDENT",      "[a-zA-Z_][a-zA-Z0-9_]*"},

    {"FLOAT_LIT",  "[0-9]+\\.[0-9]+"},
    {"INT_LIT",    "[0-9]+"},

    {"INCREMENT",  "\\+\\+"},
    {"DECREMENT",  "--"},
    {"PLUS",       "\\+"},
    {"MINUS",      "-"},
    {"MULT",       "\\*"},
    {"DIV",        "/"},
    {"MOD",        "%"},

    {"EQ",         "=="},
    {"NE",         "!="},
    {"LE",         "<="},
    {"GE",         ">="},
    {"LT",         "<"},
    {"GT",         ">"},

    {"AND",        "&&"},
    {"OR",         "\\|\\|"},
    {"NOT",        "!"},

    {"BIT_AND",    "&"},
    {"BIT_OR",     "\\|"},
    {"BIT_XOR",    "\\^"},
    {"BIT_NOT",    "~"},

    {"ASSIGN",     "="},

    {"LPAREN",     "\\("},
    {"RPAREN",     "\\)"},
    {"LBRACE",     "\\{"},
    {"RBRACE",     "\\}"},
    {"LBRACKET",   "\\["},
    {"RBRACKET",   "\\]"},

    {"SEMICOLON",  ";"},
    {"COMMA",      ","},
    {"DOT",        "\\."},
    {"COLON",      ":"},
    {"QUESTION",   "\\?"},

    {"STRING_LIT", "\"([^\"\\\\]|\\\\.)*\""},
    {"CHAR_LIT",   "'([^'\\\\]|\\\\.)'"},

    {"WHITESPACE", "[[:space:]]+"}
};

#define TOKEN_COUNT (sizeof(TOKEN_SPEC) / sizeof(TOKEN_SPEC[0]))

/* ============================================================
   FUNÇÕES AUXILIARES
   ============================================================ */

int is_word_char(char c) {
    return isalnum((unsigned char)c) || c == '_';
}


/*
 * Verifica se um token de palavra possui limites de palavra,
 * equivalente ao \b usado no Python.
 */
int valid_word_boundary(
    const char *codigo,
    int inicio,
    int fim,
    int tamanho
) {
    if (inicio > 0 && is_word_char(codigo[inicio - 1])) {
        return 0;
    }

    if (fim < tamanho && is_word_char(codigo[fim])) {
        return 0;
    }

    return 1;
}


/*
 * Faz o escape necessário para produzir uma string JSON válida.
 */
void print_json_string(const char *str) {

    putchar('"');

    while (*str) {

        switch (*str) {

            case '"':
                printf("\\\"");
                break;

            case '\\':
                printf("\\\\");
                break;

            case '\n':
                printf("\\n");
                break;

            case '\r':
                printf("\\r");
                break;

            case '\t':
                printf("\\t");
                break;

            default:
                putchar(*str);
                break;
        }

        str++;
    }

    putchar('"');
}


/* ============================================================
   GET_ATTRIBUTE
   ============================================================ */

void get_attribute(
    const char *tipo,
    const char *valor,
    char *saida,
    size_t tamanho
) {

    if (strcmp(tipo, "IDENT") == 0) {

        snprintf(
            saida,
            tamanho,
            "\"%s\"",
            valor
        );

        return;
    }

    if (strcmp(tipo, "INT_LIT") == 0) {

        snprintf(
            saida,
            tamanho,
            "%d",
            atoi(valor)
        );

        return;
    }

    if (strcmp(tipo, "FLOAT_LIT") == 0) {

        snprintf(
            saida,
            tamanho,
            "%f",
            atof(valor)
        );

        return;
    }

    if (strcmp(tipo, "STRING_LIT") == 0) {

        int len = strlen(valor);

        if (len >= 2) {

            char temp[4096];

            strncpy(
                temp,
                valor + 1,
                len - 2
            );

            temp[len - 2] = '\0';

            snprintf(
                saida,
                tamanho,
                "\"%s\"",
                temp
            );
        }

        return;
    }

    if (strcmp(tipo, "CHAR_LIT") == 0) {

        int len = strlen(valor);

        if (len >= 2) {

            char temp[256];

            strncpy(
                temp,
                valor + 1,
                len - 2
            );

            temp[len - 2] = '\0';

            snprintf(
                saida,
                tamanho,
                "\"%s\"",
                temp
            );
        }

        return;
    }

    /*
     * Python retorna None para os demais tipos.
     */
    snprintf(
        saida,
        tamanho,
        "null"
    );
}


/* ============================================================
   LEXER
   ============================================================ */

void lexer(const char *codigo) {

    int pos = 0;
    int linha = 1;
    int coluna = 1;

    int tamanho_codigo = strlen(codigo);

    while (pos < tamanho_codigo) {

        int encontrou = 0;

        char tipo[100];
        char valor[4096];

        int tamanho_match = 0;

        /*
         * Tenta cada token na mesma ordem do TOKEN_SPEC
         */
        for (int i = 0; i < TOKEN_COUNT; i++) {

            regex_t regex;
            regmatch_t match;

            int resultado = regcomp(
                &regex,
                TOKEN_SPEC[i].pattern,
                REG_EXTENDED
            );

            if (resultado != 0) {
                continue;
            }

            /*
             * Cria uma string começando na posição atual.
             */
            const char *inicio = codigo + pos;

            resultado = regexec(
                &regex,
                inicio,
                1,
                &match,
                0
            );

            if (resultado == 0 && match.rm_so == 0) {

                int inicio_match = pos + match.rm_so;
                int fim_match = pos + match.rm_eo;

                /*
                 * Tokens de palavras precisam respeitar
                 * os limites de palavra.
                 */
                if (
                    strcmp(TOKEN_SPEC[i].name, "AUTO") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "BREAK") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "CASE") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "CHAR") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "CONST") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "CONTINUE") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "DEFAULT") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "DO") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "DOUBLE") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "ELSE") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "ENUM") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "EXTERN") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "FLOAT") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "FOR") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "GOTO") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "IF") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "INT") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "LONG") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "REGISTER") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "RETURN") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "SHORT") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "SIGNED") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "SIZEOF") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "STATIC") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "STRUCT") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "SWITCH") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "TYPEDEF") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "UNION") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "UNSIGNED") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "VOID") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "VOLATILE") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "WHILE") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "BOOL") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "TRUE") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "FALSE") == 0 ||
                    strcmp(TOKEN_SPEC[i].name, "PRINT") == 0
                ) {

                    if (!valid_word_boundary(
                            codigo,
                            inicio_match,
                            fim_match,
                            tamanho_codigo
                        )) {

                        regfree(&regex);
                        continue;
                    }
                }

                tamanho_match = match.rm_eo;

                strncpy(
                    valor,
                    inicio,
                    tamanho_match
                );

                valor[tamanho_match] = '\0';

                strcpy(
                    tipo,
                    TOKEN_SPEC[i].name
                );

                encontrou = 1;

                regfree(&regex);

                break;
            }

            regfree(&regex);
        }

        /*
         * Nenhum token encontrado.
         */
        if (!encontrou) {

            fprintf(
                stderr,
                "Caractere inválido na linha %d e na coluna %d: '%c'\n",
                linha,
                coluna,
                codigo[pos]
            );

            exit(1);
        }

        int linha_inicio = linha;
        int coluna_inicio = coluna;

        pos += tamanho_match;

        /*
         * Atualização de linha e coluna.
         */
        for (int i = 0; i < tamanho_match; i++) {

            if (valor[i] == '\n') {

                linha++;
                coluna = 1;

            } else {

                coluna++;
            }
        }

        /*
         * Ignora espaços e comentários.
         */
        if (
            strcmp(tipo, "WHITESPACE") == 0 ||
            strcmp(tipo, "COMMENT") == 0 ||
            strcmp(tipo, "BLOCK_COMMENT") == 0
        ) {
            continue;
        }

        /*
         * Obtém o atributo.
         */
        char attribute[4096];

        get_attribute(
            tipo,
            valor,
            attribute,
            sizeof(attribute)
        );

        /*
         * Produz o mesmo formato JSON do Python.
         */
        printf("{");

        printf("\"token\":");
        print_json_string(tipo);

        printf(",");

        printf("\"lexeme\":");
        print_json_string(valor);

        printf(",");

        printf("\"attribute\":");
        printf("%s", attribute);

        printf(",");

        printf("\"line\":%d", linha_inicio);

        printf(",");

        printf("\"column\":%d", coluna_inicio);

        printf("}\n");
    }

    /*
     * EOF
     */
    printf("{");

    printf("\"token\":\"EOF\",");

    printf("\"lexeme\":\"\",");

    printf("\"attribute\":null,");

    printf("\"line\":%d,", linha);

    printf("\"column\":%d", coluna);

    printf("}\n");
}


/* ============================================================
   LER ARQUIVO
   ============================================================ */

char *ler_arquivo(const char *nome_arquivo) {

    FILE *arquivo = fopen(
        nome_arquivo,
        "rb"
    );

    if (arquivo == NULL) {

        fprintf(
            stderr,
            "Arquivo não encontrado: %s\n",
            nome_arquivo
        );

        exit(1);
    }

    /*
     * Vai para o final do arquivo para descobrir
     * seu tamanho.
     */
    fseek(
        arquivo,
        0,
        SEEK_END
    );

    long tamanho = ftell(arquivo);

    rewind(arquivo);

    /*
     * Reserva memória.
     */
    char *codigo = malloc(
        tamanho + 1
    );

    if (codigo == NULL) {

        fprintf(
            stderr,
            "Erro ao alocar memória.\n"
        );

        fclose(arquivo);

        exit(1);
    }

    /*
     * Lê o arquivo.
     */
    size_t lidos = fread(
        codigo,
        1,
        tamanho,
        arquivo
    );

    codigo[lidos] = '\0';

    fclose(arquivo);

    return codigo;
}


/* ============================================================
   MAIN
   ============================================================ */

int main(int argc, char *argv[]) {

    if (argc != 2) {

        printf(
            "Uso: scanner <arquivo.c>\n"
        );

        return 1;
    }

    const char *nome_arquivo = argv[1];

    char *codigo = ler_arquivo(
        nome_arquivo
    );

    lexer(codigo);

    free(codigo);

    return 0;
}