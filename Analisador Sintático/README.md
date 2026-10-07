# MINIC em C

Conversão do pipeline Python enviado para C, mantendo a organização conceitual:

- `scanner.c/.h` — análise léxica;
- `arvore.c/.h` — AST;
- `parser.c/.h` — análise sintática e construção da AST;
- `semantic.c/.h` — análise semântica, tabela de símbolos, tipos e diagnósticos `SEMxxx`;
- `minic.c` — driver da etapa semântica;
- `scanner_main.c` e `parser_main.c` — executáveis auxiliares;
- `Makefile` — compilação.

## Compilar

Linux/macOS/WSL com GCC:

```bash
make
```

Executáveis gerados:

```bash
./scanner programa.c
./parser programa.c
./minic programa.c
```

O `minic` retorna:

- `0` — programa aceito;
- `2` — erro léxico;
- `3` — erro sintático;
- `4` — programa rejeitado pela análise semântica.

## Observação

A implementação foi feita a partir dos cinco arquivos Python fornecidos. A etapa semântica preserva os tipos, escopos, tabela de símbolos, conversões implícitas e os códigos `SEM001` a `SEM015` usados pelo projeto.
