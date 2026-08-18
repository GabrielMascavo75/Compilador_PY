# Codigo de teste

# import re
# total = 2 + 3 * 4
# print (total)


import re

# Listando tokens

TOKEN_SPEC = [
    ("AUTO",       r"\bauto\b"),
    ("BREAK",      r"\bbreak\b"),
    ("CASE",       r"\bcase\b"),
    ("CHAR",       r"\bchar\b"),
    ("CONST",      r"\bconst\b"),
    ("CONTINUE",   r"\bcontinue\b"),
    ("DEFAULT",    r"\bdefault\b"),
    ("DO",         r"\bdo\b"),
    ("DOUBLE",     r"\bdouble\b"),
    ("ELSE",       r"\belse\b"),
    ("ENUM",       r"\benum\b"),
    ("EXTERN",     r"\bextern\b"),
    ("FLOAT",      r"\bfloat\b"),
    ("FOR",        r"\bfor\b"),
    ("GOTO",       r"\bgoto\b"),
    ("IF",         r"\bif\b"),
    ("INT",        r"\bint\b"),
    ("LONG",       r"\blong\b"),
    ("REGISTER",   r"\bregister\b"),
    ("RETURN",     r"\breturn\b"),
    ("SHORT",      r"\bshort\b"),
    ("SIGNED",     r"\bsigned\b"),
    ("SIZEOF",     r"\bsizeof\b"),
    ("STATIC",     r"\bstatic\b"),
    ("STRUCT",     r"\bstruct\b"),
    ("SWITCH",     r"\bswitch\b"),
    ("TYPEDEF",    r"\btypedef\b"),
    ("UNION",      r"\bunion\b"),
    ("UNSIGNED",   r"\bunsigned\b"),
    ("VOID",       r"\bvoid\b"),
    ("VOLATILE",   r"\bvolatile\b"),
    ("WHILE",      r"\bwhile\b"),

    ("IDENT",      r"\b[a-zA-Z_][a-zA-Z0-9_]*\b"),

    ("NUMBER",     r"\b[0-9]+\b"),

    ("INCREMENT",  r"\+\+"),
    ("DECREMENT",  r"--"),
    ("PLUS",       r"\+"),
    ("MINUS",      r"-"),
    ("MULT",       r"\*"),
    ("DIV",        r"/"),
    ("MOD",        r"%"),

    ("EQ",         r"=="),
    ("NE",         r"!="),
    ("LE",         r"<="),
    ("GE",         r">="),
    ("LT",         r"<"),
    ("GT",         r">"),

    ("AND",        r"&&"),
    ("OR",         r"\|\|"),
    ("NOT",        r"!"),

    ("BIT_AND",    r"&"),
    ("BIT_OR",     r"\|"),
    ("BIT_XOR",    r"\^"),
    ("BIT_NOT",    r"~"),

    ("ASSIGN",     r"="),

    ("LPAREN",     r"\("),
    ("RPAREN",     r"\)"),
    ("LBRACE",     r"\{"),
    ("RBRACE",     r"\}"),
    ("LBRACKET",   r"\["),
    ("RBRACKET",   r"\]"),

    ("SEMICOLON",  r";"),
    ("COMMA",      r","),
    ("DOT",        r"\."),
    ("COLON",      r":"),
    ("QUESTION",   r"\?"),

    ("COMMENT",    r"//[^\n]*"),
    ("BLOCK_COMMENT", r"/\*[\s\S]*?\*/"),

    ("STRING",     r'"([^"\\]|\\.)*"'),
    ("CHAR_LITERAL", r"'([^'\\]|\\.)'"),

    ("WHITESPACE", r"\s+"),
]

tok_regex = "|".join(
    f"(?P<{name}>{pattern})"
    for name, pattern in TOKEN_SPEC
)

get_token = re.compile(tok_regex).match

def lexer(codigo):
    pos = 0

    while pos < len(codigo):
        match = get_token(codigo, pos)

        if not match:
            raise SyntaxError(
                f"Caractere inválido na posição {pos}: {codigo[pos]!r}"
            )

        tipo = match.lastgroup
        valor = match.group()

        pos = match.end()

        if tipo in ("WHITESPACE", "COMMENT", "BLOCK_COMMENT"):
            continue

        print(tipo, valor)

codigo = """
int main() {
int 2total = 2 + 3 * 4;
// mostra o resultado
print(total); return 0; }
"""

lexer(codigo)