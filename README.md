# Projeto MINIC
**Feito pelos alunos:** Cauan Lemos Souza, Filipe Valle Moreira, Gabriel Macedo de Araújo Vieira e Guilherme Pinheiro  

 O MINIC é linguagem educacional inspirada em um subconjunto de C, projetada para construção incremental de um compilador. Esta especificação funciona como contrato comum entre as quatro etapas do projeto: análise sintática e AST(árvore abstrata); análise semântica e representação intermediária; geração de código e otimização.  
 A versão mínima contempla variáveis tipadas, funções, escopo léxico, vetores unidimensionais, estruturas condicionais e de repetição, operadores aritméticos, relacionais e lógicos, entrada e saída simples e comentários.
  
# Apresentação sobre o Analisador Léxico

O analisador léxico é uma das primeiras etapas do processo de compilação de um programa. Sua principal função é receber o código-fonte e dividi-lo em pequenas unidades chamadas **tokens**. Esses tokens representam elementos importantes da linguagem, como palavras reservadas, identificadores, números, operadores e símbolos.

No código apresentado, o analisador léxico deve reconhecer diferentes tipos de tokens. Por exemplo, palavras como `void`, `int`, `while`, `if`, `else` e `return` são **palavras reservadas da linguagem**. Já nomes como `mostrarMenu`, `somar`, `primeiro`, `segundo`, `opcao`, `ativo` e `total` são **identificadores**, pois foram criados pelo programador para representar funções e variáveis.

Também podemos encontrar **operadores**, como `=`, `==` e `+`. O símbolo `==`, por exemplo, é utilizado para realizar uma comparação, enquanto `=` representa uma atribuição. Os números `1`, `0`, `4` e `6` são reconhecidos como **constantes numéricas**. Além disso, símbolos como `(`, `)`, `{`, `}`, `,` e `;` também são identificados pelo analisador como tokens que ajudam a estruturar o programa.

Portanto, ao analisar esse código, o analisador léxico não precisa entender o significado completo do programa. Ele apenas identifica e classifica cada elemento encontrado. Dessa forma, o código é transformado em uma sequência organizada de tokens que posteriormente poderá ser utilizada pelo **analisador sintático**, responsável por verificar se esses elementos estão organizados de acordo com as regras da linguagem.

Em resumo, o analisador léxico funciona como uma espécie de **“separador e identificador” do código-fonte**. Ele transforma um texto escrito pelo programador em informações estruturadas, facilitando as próximas etapas do processo de compilação.

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
