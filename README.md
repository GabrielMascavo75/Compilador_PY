# Projeto MINIC
**Feito pelos alunos:** Cauan Lemos Souza, Filipe Valle Moreira, Gabriel Macedo de Araújo Vieira e Guilherme Pinheiro  

 O MINIC é linguagem educacional inspirada em um subconjunto de C, projetada para construção incremental de um compilador. Esta especificação funciona como contrato comum entre as quatro etapas do projeto: análise sintática e AST(árvore abstrata); análise semântica e representação intermediária; geração de código e otimização.  
 A versão mínima contempla variáveis tipadas, funções, escopo léxico, vetores unidimensionais, estruturas condicionais e de repetição, operadores aritméticos, relacionais e lógicos, entrada e saída simples e comentários.

  ---
  
## Analisador Léxico

O analisador léxico é uma das primeiras etapas do processo de compilação de um programa. Sua principal função é receber o código-fonte e dividi-lo em pequenas unidades chamadas **tokens**. Esses tokens representam elementos importantes da linguagem, como palavras reservadas, identificadores, números, operadores e símbolos.

No código apresentado, o analisador léxico deve reconhecer diferentes tipos de tokens. Por exemplo, palavras como `void`, `int`, `while`, `if`, `else` e `return` são **palavras reservadas da linguagem**. Já nomes como `mostrarMenu`, `somar`, `primeiro`, `segundo`, `opcao`, `ativo` e `total` são **identificadores**, pois foram criados pelo programador para representar funções e variáveis.

Também podemos encontrar **operadores**, como `=`, `==` e `+`. O símbolo `==`, por exemplo, é utilizado para realizar uma comparação, enquanto `=` representa uma atribuição. Os números `1`, `0`, `4` e `6` são reconhecidos como **constantes numéricas**. Além disso, símbolos como `(`, `)`, `{`, `}`, `,` e `;` também são identificados pelo analisador como tokens que ajudam a estruturar o programa.

Portanto, ao analisar esse código, o analisador léxico não precisa entender o significado completo do programa. Ele apenas identifica e classifica cada elemento encontrado. Dessa forma, o código é transformado em uma sequência organizada de tokens que posteriormente poderá ser utilizada pelo **analisador sintático**, responsável por verificar se esses elementos estão organizados de acordo com as regras da linguagem.

Em resumo, o analisador léxico funciona como uma espécie de **“separador e identificador” do código-fonte**. Ele transforma um texto escrito pelo programador em informações estruturadas, facilitando as próximas etapas do processo de compilação.  

---

## Análise Sintática

O analisador sintático é a etapa responsável por verificar se a sequência de tokens produzida pelo analisador léxico está organizada de acordo com a **gramática** da linguagem. Ele não se preocupa com o significado dos nomes nem com a compatibilidade de tipos; sua função é garantir que a estrutura do programa é válida.

No código apresentado, o analisador sintático deve reconhecer, por exemplo, que uma declaração de função começa com um **tipo de retorno** (`void`, `int`, `float`, `bool`, `char`), seguido de um **identificador**, uma **lista de parâmetros** entre parênteses e um **bloco** entre chaves. Da mesma forma, deve reconhecer que uma atribuição é formada por um **alvo** à esquerda do `=`, seguido de uma **expressão** à direita. Um `if` precisa de uma **condição** entre parênteses e de um **comando** como corpo; um `while` segue a mesma estrutura.

Também faz parte da análise sintática verificar a **precedência e a associatividade dos operadores**. A expressão `a + b * c`, por exemplo, deve ser interpretada como `a + (b * c)`, porque `*` tem precedência maior que `+`. Já `a = b = c` deve ser lida como `a = (b = c)`, pois a atribuição é associativa à direita.

No código, a análise sintática é implementada na classe `Parser`, que percorre a lista de tokens e constrói a AST. A precedência é tratada por níveis explícitos de funções:

```
expressao
  -> atribuicao
    -> or
      -> and
        -> igualdade
          -> relacional
            -> aditiva
              -> multiplicativa
                -> unaria
                  -> pos_fixo
                    -> primario
```

Quando o parser encontra um token inesperado — por exemplo, um `;` onde deveria haver um `)` —, ele lança uma exceção `ErroSintatico` com a linha e a coluna do problema. O programa é então rejeitado, e a AST não é construída.

Em resumo, o analisador sintático funciona como um **“verificador de estrutura”**: ele confirma que os tokens estão na ordem correta e que as construções da linguagem foram respeitadas, preparando o terreno para a análise semântica.

---

## Árvore Sintática Abstrata (AST)

A Árvore Sintática Abstrata é a representação estruturada do programa após a análise sintática. Em vez de guardar todos os tokens, ela mantém apenas as informações essenciais para as etapas seguintes, organizadas em uma hierarquia de nós.

Cada nó da AST representa uma construção da linguagem. No código, as classes que definem esses nós estão em `arvore.py`:

- `Programa` — nó raiz, contém a lista de declarações globais;
- `Funcao` — tipo de retorno, nome, parâmetros e corpo;
- `Parametro` — tipo base, nome e flag de vetor;
- `DeclVar` — tipo base, nome, tamanho (se vetor) e inicializador;
- `ListaDecl` — agrupa várias declarações do mesmo tipo;
- `Bloco` — sequência de comandos;
- `If`, `While`, `For` — comandos de controle;
- `Return`, `Break`, `Continue` — comandos de desvio;
- `Print`, `Read`, `ExprStmt` — comandos de entrada/saída e expressão;
- `Atribuicao`, `Binaria`, `Unaria`, `Chamada`, `Indice` — expressões compostas;
- `Identificador`, `Literal` — expressões atômicas.

Cada nó guarda ainda:

- `linha` e `coluna` de origem, usadas nos diagnósticos;
- `fim_linha` e `fim_coluna`, que delimitam o trecho do fonte coberto pela expressão;
- campos de anotação preenchidos pela análise semântica: `tipo` e `simbolo`.

A AST é construída pelo parser à medida que os tokens são consumidos. Por exemplo, ao reconhecer `int x = 10;`, o parser cria um nó `DeclVar` com `tipo_base="int"`, `nome="x"` e `inicializador=Literal("int", "10", 10)`. Ao reconhecer `a + b * c`, cria um nó `Binaria("+", Id("a"), Binaria("*", Id("b"), Id("c")))`.

A principal vantagem da AST é que ela **elimina a redundância** dos tokens. Parênteses, pontos e vírgulas que só serviam para guiar o parser desaparecem; o que resta é a estrutura lógica do programa. Isso torna as etapas seguintes mais simples e eficientes.

Em resumo, a AST funciona como uma **“planta estrutural”** do programa: ela mostra como as construções se relacionam, sem se prender aos detalhes sintáticos que já foram validados.

---

## Análise Semântica

A análise semântica é a etapa responsável por verificar se o programa, além de sintaticamente correto, **faz sentido**. Ela percorre a AST verificando nomes, escopos, tipos e regras contextuais, e acumula diagnósticos no catálogo `SEM001` a `SEM015`.

No código, a análise semântica é implementada na classe `semantic`, que visita cada nó da AST e mantém uma **tabela de símbolos** organizada como uma pilha de escopos. Cada símbolo registra nome, categoria (`VARIÁVEL`, `PARÂMETRO`, `VETOR`, `FUNÇÃO`), tipo, nível léxico, linha, coluna, parâmetros (para funções), tamanho (para vetores) e as flags `inicializado` e `usado`.

As principais verificações realizadas são:

- **Nomes e escopos** — um identificador só pode ser usado se foi declarado em um escopo visível. O sombreamento léxico é permitido: uma variável local pode ter o mesmo nome de uma global. Declarações duplicadas no mesmo escopo são rejeitadas (`SEM002`).

- **Tipos** — cada expressão recebe um tipo inferido, anotado em `no.tipo`. As conversões implícitas permitidas são `int` → `float`, `char` → `int` e `char` → `float`. Atribuições e retornos que violem essas regras geram `SEM003` ou `SEM009`.

- **Operadores** — `!` exige operando `bool`; `-` unário exige operando numérico; `%` exige operandos inteiros; `&&` e `||` exigem operandos `bool`. Violações geram `SEM004`.

- **Condições** — as condições de `if`, `while` e `for` devem ter tipo `bool` (`SEM005`).

- **Vetores** — o índice deve ser `int` (`SEM006`); o tamanho deve ser uma constante positiva (`SEM006`); o acesso a um não vetor gera `SEM014`; índices fora dos limites conhecidos em tempo de compilação geram `SEM006`.

- **Funções** — a aridade deve bater com a declarada (`SEM007`); os tipos dos argumentos devem ser compatíveis com os parâmetros (`SEM008`); uma função `void` não pode ser usada como valor (`SEM012`); uma função não `void` que pode terminar sem `return` gera `SEM011`.

- **Comandos de desvio** — `break` e `continue` só são válidos dentro de laços (`SEM010`).

- **Destinos** — o alvo de uma atribuição ou de um `read` deve ser uma variável ou um elemento de vetor (`SEM013`).

- **Divisão por zero** — quando o divisor é uma constante zero, o erro `SEM015` é emitido.

Os erros são acumulados e ordenados por linha e coluna, e o programa é aceito apenas se a lista estiver vazia. Caso contrário, o driver `minic.py` retorna `4`.

Em resumo, a análise semântica funciona como um **“verificador de coerência”**: ela garante que os nomes existem, que os tipos são compatíveis e que as construções da linguagem são usadas de acordo com as regras contextuais, completando o processo de validação iniciado pelo léxico e pela sintaxe.

---

## Resumo das Etapas

| Etapa | Arquivo | Responsabilidade | Códigos de erro |
|-------|---------|------------------|-----------------|
| Análise léxica | `scanner.py` | Dividir o código em tokens | `UNKNOWN_SYMBOL`, `UNTERMINATED_STRING`, `UNTERMINATED_CHAR`, `UNTERMINATED_BLOCK_COMMENT` |
| Análise sintática | `parser.py` | Verificar a estrutura conforme a gramática | `ErroSintatico` |
| Árvore abstrata | `arvore.py` | Representar a estrutura lógica do programa | — |
| Análise semântica | `semantic.py` | Verificar nomes, escopos, tipos e regras contextuais | `SEM001` a `SEM015` |
| Driver | `minic.py` | Orquestrar as etapas e reportar o resultado | `0`, `2`, `3`, `4` |

Cada etapa depende da anterior: o parser precisa dos tokens, a AST precisa da estrutura validada, e a análise semântica precisa da AST. Juntas, elas transformam o texto-fonte em um programa verificado, pronto para as fases seguintes da compilação.
