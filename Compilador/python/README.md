# MINIC em Python

Implementação de referência do compilador MINIC em Python, organizada em módulos que espelham as fases clássicas de um compilador:

- `scanner.py` — análise léxica;
- `arvore.py` — definição dos nós da AST;
- `parser.py` — análise sintática e construção da AST;
- `semantic.py` — análise semântica, tabela de símbolos, tipos e diagnósticos `SEMxxx`;
- `minic.py` — driver da etapa semântica.

## Compilar

Python não exige etapa de compilação. Basta ter o interpretador instalado (Python 3.8 ou superior) e executar os módulos diretamente:

```bash
python3 scanner.py programa.c
python3 parser.py programa.c
python3 minic.py programa.c
```

Também é possível tornar o driver executável:

```bash
chmod +x minic.py
./minic.py programa.c
```

## Executáveis / Pontos de entrada

O projeto não gera binários; cada arquivo com bloco `if __name__ == "__main__":` funciona como um pequeno programa:

```bash
python3 scanner.py programa.c   # imprime os tokens em JSON
python3 parser.py programa.c    # imprime a AST ou a mensagem de rejeição
python3 minic.py programa.c     # executa scanner + parser + semântica
```

O `minic.py` retorna:

- `0` — programa aceito;
- `1` — erro de uso ou arquivo não encontrado;
- `2` — erro léxico;
- `3` — erro sintático;
- `4` — programa rejeitado pela análise semântica.

## Organização dos módulos

### `scanner.py`

Usa expressões regulares para reconhecer a linguagem. A lista `TOKEN_SPEC` define, em ordem de prioridade, palavras reservadas, comentários, strings, caracteres, identificadores, números, operadores e delimitadores. Cada token válido é emitido como um objeto JSON com `token`, `lexeme`, `attribute`, `line` e `column`. Erros léxicos (`UNKNOWN_SYMBOL`, `UNTERMINATED_STRING`, `UNTERMINATED_CHAR`, `UNTERMINATED_BLOCK_COMMENT`) também são emitidos em JSON e fazem o scanner retornar `2`.

### `arvore.py`

Contém as classes que representam a Árvore Sintática Abstrata: `Programa`, `Funcao`, `Parametro`, `DeclVar`, `ListaDecl`, `Bloco`, `If`, `While`, `For`, `Return`, `Break`, `Continue`, `Print`, `Read`, `ExprStmt`, `Atribuicao`, `Binaria`, `Unaria`, `Chamada`, `Indice`, `Identificador` e `Literal`.

Cada nó guarda:

- seus filhos;
- `linha` e `coluna` de origem;
- `fim_linha` e `fim_coluna`, usados para recortar o trecho do fonte nas mensagens;
- campos de anotação preenchidos pela análise semântica: `tipo` e `simbolo`.

O método `__str__` de cada classe reproduz o formato textual que o parser imprime, permitindo inspecionar a AST sem ferramentas extras.

### `parser.py`

Implementa um analisador descendente recursivo. O método `obter_tokens` executa o `scanner.py` como subprocesso, lê a saída JSON e transforma erros léxicos em exceções `ErroLexico`. A classe `Parser` percorre a lista de tokens e constrói a AST.

A precedência de operadores é tratada por níveis explícitos:

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

O parâmetro `estrito` controla o comportamento do parser:

- `estrito=True` — alvo de atribuição ou de `read` que não seja variável ou elemento de vetor é erro sintático;
- `estrito=False` — a AST é construída e o diagnóstico `SEM013` fica a cargo da análise semântica. É o modo usado pelo `minic.py`.

### `semantic.py`

Percorre a AST verificando nomes, escopos, tipos e regras contextuais. Os diagnósticos seguem o catálogo `SEM001` a `SEM015`:

| Código | Significado |
|--------|-------------|
| `SEM001` | identificador não declarado |
| `SEM002` | declaração duplicada |
| `SEM003` | incompatibilidade de atribuição/conversão |
| `SEM004` | operando de tipo inválido para o operador |
| `SEM005` | condição não booleana |
| `SEM006` | índice (ou tamanho) de vetor inválido |
| `SEM007` | aridade incorreta |
| `SEM008` | tipo de argumento incompatível |
| `SEM009` | retorno incompatível |
| `SEM010` | `break`/`continue` fora de laço |
| `SEM011` | possível queda de função não `void` |
| `SEM012` | chamada `void` usada como valor |
| `SEM013` | destino não atribuível |
| `SEM014` | uso inválido de símbolo ou tipo |
| `SEM015` | divisão por zero constante |

A tabela de símbolos é uma pilha de escopos (`TabelaSimbolos`), permitindo sombreamento léxico. Cada símbolo registra nome, categoria (`VARIÁVEL`, `PARÂMETRO`, `VETOR`, `FUNÇÃO`), tipo, nível, linha, coluna, parâmetros (para funções), tamanho (para vetores), além das flags `inicializado` e `usado`.

As conversões implícitas permitidas são:

- `int` → `float`;
- `char` → `int`;
- `char` → `float` (composição `char` → `int` → `float`).

A análise também verifica cobertura de retornos (`motivo_queda`), laços infinitos (`laco_infinito`) e presença de `break` que sai do laço atual (`contem_break`).

### `minic.py`

Driver da etapa semântica. Chama `analisar_sintaxe` com `estrito=False`, lê o fonte em UTF-8 e executa `semantic(fonte).analisar(ast)`. Os erros são ordenados por linha e coluna e impressos com o separador `\r\n`, para compatibilidade com os gabaritos da suíte de testes. A saída termina com uma linha de resumo:

```
Análise semântica concluída: N erros; programa aceito/rejeitado.
```

## Por que Python não possui arquivos `.h`

Em C, os arquivos `.h` (headers) existem porque o compilador precisa conhecer a **declaração** de tipos, funções e variáveis antes de usá-los. Como C é uma linguagem de **tipagem estática** e compilada em etapas separadas, cada unidade de tradução (`.c`) é compilada isoladamente; o header funciona como um contrato que informa ao compilador:

- o nome e a assinatura das funções;
- o layout das `struct`s e `union`s;
- o tipo das variáveis globais;
- macros e constantes.

Sem o header, o compilador não saberia, por exemplo, que `int soma(int, int);` existe e qual é o tipo de retorno, nem poderia verificar se uma chamada está correta.

Python, por outro lado, tem **tipagem dinâmica**: os tipos são associados aos **valores em tempo de execução**, não às variáveis em tempo de compilação. Isso traz consequências práticas:

1. **Não há verificação de tipos em tempo de compilação.** O interpretador descobre o tipo de um objeto quando a operação é executada. Portanto, não existe a necessidade de um arquivo que declare antecipadamente os tipos para o compilador validar.

2. **A resolução de nomes é feita em tempo de execução.** Quando um módulo faz `from arvore import Programa`, o import é resolvido pelo interpretador no momento da execução (ou na primeira importação). O módulo `arvore.py` já contém as definições necessárias; não é preciso um `.h` separado com as declarações, porque o próprio `.py` é o contrato.

3. **Não há separação entre declaração e definição.** Em C, é comum declarar em `.h` e definir em `.c`. Em Python, a definição da classe ou função **é** a declaração: ao executar `class Programa: ...`, o nome `Programa` passa a existir no namespace do módulo. Não há duas entidades distintas (declaração vs. definição) para reconciliar.

4. **Não há compilação separada por unidade de tradução.** Python compila para bytecode (`.pyc`) o módulo inteiro, mas a importação é dinâmica e o namespace é construído em tempo de execução. Não existe o conceito de "unidade de tradução" que precise de um header para enxergar símbolos de outro arquivo.

5. **A verificação de tipos é opcional e externa.** Ferramentas como `mypy`, `pyright` e `pyre` fazem análise estática usando **type hints** (`def soma(a: int, b: int) -> int: ...`), mas esses hints ficam no próprio `.py` e não são exigidos pelo interpretador. Eles funcionam como documentação verificável, não como um header obrigatório.

6. **O acoplamento é feito por importação, não por inclusão textual.** Em C, `#include "arvore.h"` insere o conteúdo do header no arquivo `.c`. Em Python, `import arvore` cria uma referência ao módulo já carregado; não há cópia textual de declarações. Isso elimina a necessidade de um arquivo separado só para expor a interface.

Em resumo: **o `.h` existe em C para dar ao compilador a informação de tipos e assinaturas que ele precisa antes de compilar cada arquivo isoladamente.** Em Python, como os tipos são dinâmicos, a resolução de nomes é feita em tempo de execução e o próprio módulo `.py` carrega suas definições, não há essa etapa de "declaração prévia" — logo, não há `.h`. O equivalente funcional mais próximo são os **módulos** (`.py`) que expõem nomes via `import`, e, quando se quer verificação estática, os **type hints** e arquivos de stubs (`.pyi`), que são opcionais e não substituem o módulo em si.

## Observação

A implementação em Python é a referência conceitual do projeto. A versão em C descrita no `README.md` traduz a mesma organização: cada fase vira um par `.c`/`.h`, a tabela de símbolos é implementada com estruturas próprias e os diagnósticos `SEM001` a `SEM015` são preservados. A diferença fundamental é que, em C, a separação entre interface (`.h`) e implementação (`.c`) é obrigatória para a compilação separada; em Python, essa separação é uma escolha de estilo, não uma exigência da linguagem.
