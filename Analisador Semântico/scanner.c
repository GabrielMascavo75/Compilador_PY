/*
 * scanner.c — Analisador léxico 
 */
#include "util.h"
#include "scanner.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static int eh_digito(unsigned char c) { return c >= '0' && c <= '9'; }

static int eh_letra(unsigned char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}


/* Leitura do código-fonte */

char *fonte;        /* texto normalizado */
size_t tam_fonte;   /* tamanho em bytes */

int ler_fonte(const char *nome)
{
    struct stat info;
    FILE *f;
    char *bruto;
    size_t tam, lidos, i, j;

    /* Como o os.path.isfile do Python: diretórios não contam. */
    if (stat(nome, &info) != 0 || !S_ISREG(info.st_mode))
        return 0;

    f = fopen(nome, "rb");
    if (!f)
        return 0;

    tam = (size_t)info.st_size;
    bruto = xmalloc(tam + 1);
    lidos = fread(bruto, 1, tam, f);
    fclose(f);

    /* Normaliza as quebras de linha. */
    fonte = xmalloc(lidos + 1);
    for (i = 0, j = 0; i < lidos; i++) {
        if (bruto[i] == '\r') {
            fonte[j++] = '\n';
            if (i + 1 < lidos && bruto[i + 1] == '\n')
                i++;
        } else {
            fonte[j++] = bruto[i];
        }
    }
    fonte[j] = '\0';
    tam_fonte = j;
    free(bruto);
    return 1;
}

int utf8_valido(const unsigned char *s, size_t n)
{
    size_t i = 0;
    while (i < n) {
        unsigned char c = s[i];
        size_t k, len;
        unsigned long cp;

        if (c < 0x80) { i++; continue; }
        else if (c >= 0xC2 && c <= 0xDF) { len = 2; cp = c & 0x1F; }
        else if (c >= 0xE0 && c <= 0xEF) { len = 3; cp = c & 0x0F; }
        else if (c >= 0xF0 && c <= 0xF4) { len = 4; cp = c & 0x07; }
        else return 0;

        if (i + len > n)
            return 0;
        for (k = 1; k < len; k++) {
            if ((s[i + k] & 0xC0) != 0x80)
                return 0;
            cp = (cp << 6) | (s[i + k] & 0x3F);
        }
        if ((len == 3 && cp < 0x800) || (len == 4 && cp < 0x10000)
            || cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF))
            return 0;
        i += len;
    }
    return 1;
}

static size_t tam_caractere(const unsigned char *s)
{
    if (s[0] < 0x80) return 1;
    if (s[0] < 0xE0) return 2;
    if (s[0] < 0xF0) return 3;
    return 4;
}

static unsigned long decodificar(const unsigned char *s)
{
    size_t len = tam_caractere(s), k;
    unsigned long cp;

    if (len == 1) return s[0];
    cp = s[0] & (len == 2 ? 0x1F : len == 3 ? 0x0F : 0x07);
    for (k = 1; k < len; k++)
        cp = (cp << 6) | (s[k] & 0x3F);
    return cp;
}


/* Geração de tokens */


const char *NOME_TOKEN[] = {
    "INT", "FLOAT", "BOOL", "CHAR", "VOID", "IF", "ELSE", "WHILE", "FOR",
    "RETURN", "BREAK", "CONTINUE", "TRUE", "FALSE", "PRINT", "READ",
    "STRING_LIT", "CHAR_LIT", "IDENT", "FLOAT_LIT", "INT_LIT",
    "EQ", "NE", "LE", "GE", "AND", "OR",
    "PLUS", "MINUS", "STAR", "SLASH", "PERCENT",
    "LT", "GT", "NOT", "ASSIGN",
    "LPAREN", "RPAREN", "LBRACE", "RBRACE", "LBRACKET", "RBRACKET",
    "SEMICOLON", "COMMA", "DOT",
    "EOF", "IGNORAR"
};

static const char *PALAVRAS[] = {
    "int", "float", "bool", "char", "void", "if", "else", "while", "for",
    "return", "break", "continue", "true", "false", "print", "read"
};
#define N_PALAVRAS ((int)(sizeof PALAVRAS / sizeof PALAVRAS[0]))


Token *tokens;
int n_tokens;
static int cap_tokens;
char *msg_erro_lexico;

static void adicionar_token(TipoToken tipo, size_t ini, size_t fim,
                            int linha, int coluna)
{
    Token *t;
    if (n_tokens == cap_tokens) {
        cap_tokens = cap_tokens ? cap_tokens * 2 : 256;
        tokens = xrealloc(tokens, (size_t)cap_tokens * sizeof *tokens);
    }
    t = &tokens[n_tokens++];
    t->tipo = tipo;
    t->lexema = xstrndup(fonte + ini, fim - ini);
    t->ini = ini;
    t->fim = fim;
    t->linha = linha;
    t->coluna = coluna;
}

static int eh_espaco(unsigned long cp)
{
    return cp == ' ' || (cp >= 0x09 && cp <= 0x0D) || (cp >= 0x1C && cp <= 0x1F)
        || cp == 0x85 || cp == 0xA0 || cp == 0x1680
        || (cp >= 0x2000 && cp <= 0x200A) || cp == 0x2028 || cp == 0x2029
        || cp == 0x202F || cp == 0x205F || cp == 0x3000;
}

static int eh_palavra_em(size_t pos)
{
    const unsigned char *s = (const unsigned char *)fonte;
    if (pos >= tam_fonte)
        return 0;
    if (s[pos] < 0x80)
        return eh_letra(s[pos]) || eh_digito(s[pos]);
    return !eh_espaco(decodificar(s + pos));
}

static int eh_palavra_antes(size_t pos)
{
    const unsigned char *s = (const unsigned char *)fonte;
    if (pos == 0)
        return 0;
    pos--;
    while (pos > 0 && (s[pos] & 0xC0) == 0x80)   /* volta ao início do caractere */
        pos--;
    return eh_palavra_em(pos);
}

static int eh_escape(char c)
{
    return c == 'n' || c == 't' || c == '\\' || c == '\'' || c == '"';
}

/* Analisa o fonte inteiro. Devolve 1 em caso de sucesso; em caso de erro,
 * guarda a mensagem do PRIMEIRO erro e devolve 0. */
int lexer(void)
{
    const unsigned char *s = (const unsigned char *)fonte;
    size_t pos = 0;
    int linha = 1, coluna = 1;

    while (pos < tam_fonte) {
        size_t ini = pos, p;
        TipoToken tipo = T_IGNORAR;
        const char *erro = NULL;
        unsigned char c = s[pos];
        unsigned char prox = pos + 1 < tam_fonte ? s[pos + 1] : 0;
        int i;

        /* Palavras reservadas e identificadores */
        if (eh_letra(c)) {
            int palavra = -1;
            if (!eh_palavra_antes(pos)) {
                for (i = 0; i < N_PALAVRAS; i++) {
                    size_t len = strlen(PALAVRAS[i]);
                    if (pos + len <= tam_fonte
                        && memcmp(fonte + pos, PALAVRAS[i], len) == 0
                        && !eh_palavra_em(pos + len)) {
                        palavra = i;
                        break;
                    }
                }
            }
            if (palavra >= 0) {
                tipo = (TipoToken)palavra;
                pos += strlen(PALAVRAS[palavra]);
            } else {
                tipo = T_IDENT;
                while (pos < tam_fonte && (eh_letra(s[pos]) || eh_digito(s[pos])))
                    pos++;
            }
        }
        /* Comentários */
        else if (c == '/' && prox == '/') {
            while (pos < tam_fonte && s[pos] != '\n')
                pos++;
        }
        else if (c == '/' && prox == '*') {
            const char *fim = NULL;
            for (p = pos + 2; p + 1 < tam_fonte; p++) {
                if (s[p] == '*' && s[p + 1] == '/') {
                    fim = fonte + p;
                    break;
                }
            }
            if (fim) {
                pos = (size_t)(fim - fonte) + 2;
            } else {
                pos = tam_fonte;
                erro = "UNTERMINATED_BLOCK_COMMENT";
            }
        }
        /* Cadeias */
        else if (c == '"') {
            int ok = 0;
            p = pos + 1;
            while (p < tam_fonte) {
                if (s[p] == '"') { ok = 1; p++; break; }
                if (s[p] == '\\') {
                    if (p + 1 < tam_fonte && eh_escape((char)s[p + 1])) { p += 2; continue; }
                    break;  
                }
                p++;
            }
            if (ok) {
                tipo = T_STRING_LIT;
                pos = p;
            } else {
                p = pos + 1;
                while (p < tam_fonte && s[p] != '"' && s[p] != '\n' && s[p] != ')')
                    p++;
                pos = p;
                erro = "UNTERMINATED_STRING";
            }
        }
        /* Caracteres */
        else if (c == '\'') {
            int ok = 0;
            p = pos + 1;
            if (p < tam_fonte) {
                int valido = 1;
                if (s[p] == '\\') {
                    if (p + 1 < tam_fonte && eh_escape((char)s[p + 1])) p += 2;
                    else valido = 0;
                } else if (s[p] != '\'') {
                    p += tam_caractere(s + p);  
                } else {
                    valido = 0;
                }
                if (valido && p < tam_fonte && s[p] == '\'') {
                    ok = 1;
                    p++;
                }
            }
            if (ok) {
                tipo = T_CHAR_LIT;
                pos = p;
            } else {
                p = pos + 1;
                while (p < tam_fonte && s[p] != '\n')
                    p++;
                pos = p;
                erro = "UNTERMINATED_CHAR";
            }
        }
        /* Números */
        else if (eh_digito(c)) {
            while (pos < tam_fonte && eh_digito(s[pos]))
                pos++;
            tipo = T_INT_LIT;
            if (pos + 1 < tam_fonte && s[pos] == '.' && eh_digito(s[pos + 1])) {
                pos++;
                while (pos < tam_fonte && eh_digito(s[pos]))
                    pos++;
                tipo = T_FLOAT_LIT;
            }
        }
        /* Operadores de dois caracteres */
        else if (c == '=' && prox == '=') { tipo = T_EQ;  pos += 2; }
        else if (c == '!' && prox == '=') { tipo = T_NE;  pos += 2; }
        else if (c == '<' && prox == '=') { tipo = T_LE;  pos += 2; }
        else if (c == '>' && prox == '=') { tipo = T_GE;  pos += 2; }
        else if (c == '&' && prox == '&') { tipo = T_AND; pos += 2; }
        else if (c == '|' && prox == '|') { tipo = T_OR;  pos += 2; }
        /* Operadores e delimitadores de um caractere */
        else if (strchr("+-*/%<>!=(){}[];,.", c) && c != '\0') {
            static const char simbolos[] = "+-*/%<>!=(){}[];,.";
            static const TipoToken tipos[] = {
                T_PLUS, T_MINUS, T_STAR, T_SLASH, T_PERCENT, T_LT, T_GT,
                T_NOT, T_ASSIGN, T_LPAREN, T_RPAREN, T_LBRACE, T_RBRACE,
                T_LBRACKET, T_RBRACKET, T_SEMICOLON, T_COMMA, T_DOT
            };
            tipo = tipos[strchr(simbolos, c) - simbolos];
            pos++;
        }
        /* Espaços */
        else if (eh_espaco(decodificar(s + pos))) {
            while (pos < tam_fonte && eh_espaco(decodificar(s + pos)))
                pos += tam_caractere(s + pos);
        }
        /* Símbolo desconhecido */
        else {
            pos += tam_caractere(s + pos);
            erro = "UNKNOWN_SYMBOL";
        }

        if (erro) {
            char *lexema = xstrndup(fonte + ini, pos - ini);
            msg_erro_lexico = fmt("%s ('%s') na linha %d, coluna %d",
                                  erro, lexema, linha, coluna);
            return 0;
        }

        if (tipo != T_IGNORAR)
            adicionar_token(tipo, ini, pos, linha, coluna);

        /* Atualiza linha e coluna (contando caracteres, não bytes). */
        for (p = ini; p < pos; p += tam_caractere(s + p)) {
            if (s[p] == '\n') {
                linha++;
                coluna = 1;
            } else {
                coluna++;
            }
        }
    }

    adicionar_token(T_EOF, tam_fonte, tam_fonte, linha, coluna);
    return 1;
}
