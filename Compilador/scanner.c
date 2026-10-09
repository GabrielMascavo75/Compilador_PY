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

// Sequências de escape válidas
#define ESCAPE "\\\\(?:n|t|\\\\|'|\")"

TokenSpec TOKEN_SPEC[] = {

    // =========================
    // Palavras reservadas
    // =========================
    {"INT",       "\\bint\\b"},
    {"FLOAT",     "\\bfloat\\b"},
    {"BOOL",      "\\bbool\\b"},
    {"CHAR",      "\\bchar\\b"},
    {"VOID",      "\\bvoid\\b"},
    {"IF",        "\\bif\\b"},
    {"ELSE",      "\\belse\\b"},
    {"WHILE",     "\\bwhile\\b"},
    {"FOR",       "\\bfor\\b"},
    {"RETURN",    "\\breturn\\b"},
    {"BREAK",     "\\bbreak\\b"},
    {"CONTINUE",  "\\bcontinue\\b"},
    {"TRUE",      "\\btrue\\b"},
    {"FALSE",     "\\bfalse\\b"},
    {"PRINT",     "\\bprint\\b"},
    {"READ",      "\\bread\\b"},

    // =========================
    // Comentários
    // =========================
    {"COMMENT",                "//[^\\n]*"},
    {"BLOCK_COMMENT",          "/\\*([^*]|\\*+[^*/])*\\*+/"},
    {"UNTERMINATED_BLOCK_COMMENT", "/\\*([^*]|\\*+[^*/])*$"},

    // =========================
    // Strings e caracteres
    // =========================
    {"STRING_LIT",             "\"([^\"\\\\]|\\\\[nt\\\\'\"])*\""},
    {"CHAR_LIT",               "'([^'\\\\]|\\\\[nt\\\\'\"])'"},
    {"UNTERMINATED_STRING",    "\"[^\"\n)]*"},
    {"UNTERMINATED_CHAR",      "'[^'\n]*"},

    // =========================
    // Identificadores e números
    // =========================
    {"IDENT",                  "[a-zA-Z_][a-zA-Z0-9_]*"},
    {"FLOAT_LIT",              "[0-9]+\\.[0-9]+"},
    {"INT_LIT",                "[0-9]+"},

    // =========================
    // Operadores
    // =========================
    {"EQ",                     "=="},
    {"NE",                     "!="},
    {"LE",                     "<="},
    {"GE",                     ">="},
    {"AND",                    "&&"},
    {"OR",                     "\\|\\|"},
    {"PLUS",                   "\\+"},
    {"MINUS",                  "-"},
    {"STAR",                   "\\*"},
    {"SLASH",                  "/"},
    {"PERCENT",                "%"},
    {"LT",                     "<"},
    {"GT",                     ">"},
    {"NOT",                    "!"},
    {"ASSIGN",                 "="},

    // =========================
    // Delimitadores
    // =========================
    {"LPAREN",                 "\\("},
    {"RPAREN",                 "\\)"},
    {"LBRACE",                 "\\{"},
    {"RBRACE",                 "\\}"},
    {"LBRACKET",               "\\["},
    {"RBRACKET",               "\\]"},
    {"SEMICOLON",              ";"},
    {"COMMA",                  ","},
    {"DOT",                    "\\."},

    // =========================
    // Espaços e quebras de linha
    // =========================
    {"WHITESPACE",             "\\s+"}
};

#define TOKEN_COUNT (sizeof(TOKEN_SPEC) / sizeof(TOKEN_SPEC[0]))

/* ============================================================
   ESTRUTURA DE ERRO
   ============================================================ */

typedef struct {
    char error[100];
    char lexeme[4096];
    int line;
    int column;
} Error;

/* ============================================================
   FUNÇÕES AUXILIARES
   ============================================================ */

int is_word_char(char c) {
    return isalnum((unsigned char)c) || c == '_';
}

int valid_word_boundary(const char *codigo, int inicio, int fim, int tamanho) {
    if (inicio > 0 && is_word_char(codigo[inicio - 1])) {
        return 0;
    }
    if (fim < tamanho && is_word_char(codigo[fim])) {
        return 0;
    }
    return 1;
}

void print_json_string(const char *str) {
    putchar('"');
    while (*str) {
        switch (*str) {
            case '"':  printf("\\\""); break;
            case '\\': printf("\\\\"); break;
            case '\n': printf("\\n"); break;
            case '\r': printf("\\r"); break;
            case '\t': printf("\\t"); break;
            default:   putchar(*str); break;
        }
        str++;
    }
    putchar('"');
}

int is_reserved_word(const char *name) {
    const char *reserved[] = {
        "INT", "FLOAT", "BOOL", "CHAR", "VOID", "IF", "ELSE",
        "WHILE", "FOR", "RETURN", "BREAK", "CONTINUE", "TRUE",
        "FALSE", "PRINT", "READ"
    };
    
    for (int i = 0; i < sizeof(reserved)/sizeof(reserved[0]); i++) {
        if (strcmp(name, reserved[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/* ============================================================
   GET_ATTRIBUTE
   ============================================================ */

void get_attribute(const char *tipo, const char *valor, char *saida, size_t tamanho) {
    if (strcmp(tipo, "IDENT") == 0) {
        snprintf(saida, tamanho, "\"%s\"", valor);
        return;
    }
    
    if (strcmp(tipo, "INT_LIT") == 0) {
        snprintf(saida, tamanho, "%d", atoi(valor));
        return;
    }
    
    if (strcmp(tipo, "FLOAT_LIT") == 0) {
        snprintf(saida, tamanho, "%f", atof(valor));
        return;
    }
    
    if (strcmp(tipo, "STRING_LIT") == 0 || strcmp(tipo, "CHAR_LIT") == 0) {
        int len = strlen(valor);
        if (len >= 2) {
            char temp[4096];
            strncpy(temp, valor + 1, len - 2);
            temp[len - 2] = '\0';
            snprintf(saida, tamanho, "\"%s\"", temp);
        }
        return;
    }
    
    snprintf(saida, tamanho, "null");
}

/* ============================================================
   IMPRIMIR ERRO
   ============================================================ */

void print_error(const char *error_type, const char *lexeme, int line, int column) {
    printf("{\"error\":");
    print_json_string(error_type);
    printf(",\"lexeme\":");
    print_json_string(lexeme);
    printf(",\"line\":%d,\"column\":%d}\n", line, column);
}

/* ============================================================
   LEXER
   ============================================================ */

int lexer(const char *codigo) {
    int pos = 0;
    int linha = 1;
    int coluna = 1;
    int tamanho_codigo = strlen(codigo);
    int teve_erro = 0;
    
    // Compila todas as regex uma vez
    regex_t regexes[TOKEN_COUNT];
    int regex_compiled[TOKEN_COUNT];
    
    for (int i = 0; i < TOKEN_COUNT; i++) {
        regex_compiled[i] = 0;
        if (regcomp(&regexes[i], TOKEN_SPEC[i].pattern, REG_EXTENDED) == 0) {
            regex_compiled[i] = 1;
        }
    }
    
    while (pos < tamanho_codigo) {
        int encontrou = 0;
        int tamanho_match = 0;
        int match_index = -1;
        char valor[4096] = {0};
        
        // Tenta cada token
        for (int i = 0; i < TOKEN_COUNT; i++) {
            if (!regex_compiled[i]) continue;
            
            regmatch_t match;
            const char *inicio = codigo + pos;
            
            if (regexec(&regexes[i], inicio, 1, &match, 0) == 0 && match.rm_so == 0) {
                int inicio_match = pos + match.rm_so;
                int fim_match = pos + match.rm_eo;
                
                // Verifica limites de palavra para palavras reservadas
                if (is_reserved_word(TOKEN_SPEC[i].name)) {
                    if (!valid_word_boundary(codigo, inicio_match, fim_match, tamanho_codigo)) {
                        continue;
                    }
                }
                
                tamanho_match = match.rm_eo;
                strncpy(valor, inicio, tamanho_match);
                valor[tamanho_match] = '\0';
                match_index = i;
                encontrou = 1;
                break;
            }
        }
        
        // Símbolo desconhecido
        if (!encontrou) {
            print_error("UNKNOWN_SYMBOL", (char[]){codigo[pos], '\0'}, linha, coluna);
            teve_erro = 1;
            
            if (codigo[pos] == '\n') {
                linha++;
                coluna = 1;
            } else {
                coluna++;
            }
            pos++;
            continue;
        }
        
        char *tipo = (char *)TOKEN_SPEC[match_index].name;
        int linha_inicio = linha;
        int coluna_inicio = coluna;
        
        pos += tamanho_match;
        
        // Atualiza linha e coluna
        for (int i = 0; i < tamanho_match; i++) {
            if (valor[i] == '\n') {
                linha++;
                coluna = 1;
            } else {
                coluna++;
            }
        }
        
        // =========================
        // Erro: string não terminada
        // =========================
        if (strcmp(tipo, "UNTERMINATED_STRING") == 0) {
            print_error("UNTERMINATED_STRING", valor, linha_inicio, coluna_inicio);
            teve_erro = 1;
            continue;
        }
        
        // =========================
        // Erro: caractere não terminado
        // =========================
        if (strcmp(tipo, "UNTERMINATED_CHAR") == 0) {
            print_error("UNTERMINATED_CHAR", valor, linha_inicio, coluna_inicio);
            teve_erro = 1;
            continue;
        }
        
        // =========================
        // Erro: comentário não terminado
        // =========================
        if (strcmp(tipo, "UNTERMINATED_BLOCK_COMMENT") == 0) {
            print_error("UNTERMINATED_BLOCK_COMMENT", valor, linha_inicio, coluna_inicio);
            teve_erro = 1;
            continue;
        }
        
        // =========================
        // Ignora espaços e comentários
        // =========================
        if (strcmp(tipo, "WHITESPACE") == 0 ||
            strcmp(tipo, "COMMENT") == 0 ||
            strcmp(tipo, "BLOCK_COMMENT") == 0) {
            continue;
        }
        
        // =========================
        // Token válido
        // =========================
        char attribute[4096];
        get_attribute(tipo, valor, attribute, sizeof(attribute));
        
        printf("{");
        printf("\"token\":"); print_json_string(tipo); printf(",");
        printf("\"lexeme\":"); print_json_string(valor); printf(",");
        printf("\"attribute\":%s,", attribute);
        printf("\"line\":%d,", linha_inicio);
        printf("\"column\":%d", coluna_inicio);
        printf("}\n");
    }
    
    // Libera as regex
    for (int i = 0; i < TOKEN_COUNT; i++) {
        if (regex_compiled[i]) {
            regfree(&regexes[i]);
        }
    }
    
    // EOF
    printf("{\"token\":\"EOF\",\"lexeme\":\"\",\"attribute\":null,\"line\":%d,\"column\":%d}\n", linha, coluna);
    
    return teve_erro ? 2 : 0;
}

/* ============================================================
   LER ARQUIVO
   ============================================================ */

char *ler_arquivo(const char *nome_arquivo) {
    FILE *arquivo = fopen(nome_arquivo, "rb");
    if (arquivo == NULL) {
        return NULL;
    }
    
    fseek(arquivo, 0, SEEK_END);
    long tamanho = ftell(arquivo);
    rewind(arquivo);
    
    char *codigo = malloc(tamanho + 1);
    if (codigo == NULL) {
        fclose(arquivo);
        return NULL;
    }
    
    size_t lidos = fread(codigo, 1, tamanho, arquivo);
    codigo[lidos] = '\0';
    
    fclose(arquivo);
    return codigo;
}

/* ============================================================
   MAIN
   ============================================================ */

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: python scanner.py <arquivo.c>\n");
        return 1;
    }
    
    const char *nome_arquivo = argv[1];
    char *codigo = ler_arquivo(nome_arquivo);
    
    if (codigo == NULL) {
        fprintf(stderr, "Arquivo não encontrado: %s\n", nome_arquivo);
        return 1;
    }
    
    int resultado = lexer(codigo);
    free(codigo);
    
    return resultado;
}
