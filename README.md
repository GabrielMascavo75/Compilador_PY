# Compilador_PY
Compilador em python para receber linguagemC  
**Feito pelos alunos:** Cauan Lemos Souza, Filipe Valle Moreira, Gabriel Macedo de Araújo Vieira e Guilherme Pinheiro
  
# Apresentação sobre o Analisador Léxico

O analisador léxico é uma das primeiras etapas do processo de compilação de um programa. Sua principal função é receber o código-fonte e dividi-lo em pequenas unidades chamadas **tokens**. Esses tokens representam elementos importantes da linguagem, como palavras reservadas, identificadores, números, operadores e símbolos.

No código apresentado, o analisador léxico deve reconhecer diferentes tipos de tokens. Por exemplo, palavras como `void`, `int`, `while`, `if`, `else` e `return` são **palavras reservadas da linguagem**. Já nomes como `mostrarMenu`, `somar`, `primeiro`, `segundo`, `opcao`, `ativo` e `total` são **identificadores**, pois foram criados pelo programador para representar funções e variáveis.

Também podemos encontrar **operadores**, como `=`, `==` e `+`. O símbolo `==`, por exemplo, é utilizado para realizar uma comparação, enquanto `=` representa uma atribuição. Os números `1`, `0`, `4` e `6` são reconhecidos como **constantes numéricas**. Além disso, símbolos como `(`, `)`, `{`, `}`, `,` e `;` também são identificados pelo analisador como tokens que ajudam a estruturar o programa.

Portanto, ao analisar esse código, o analisador léxico não precisa entender o significado completo do programa. Ele apenas identifica e classifica cada elemento encontrado. Dessa forma, o código é transformado em uma sequência organizada de tokens que posteriormente poderá ser utilizada pelo **analisador sintático**, responsável por verificar se esses elementos estão organizados de acordo com as regras da linguagem.

Em resumo, o analisador léxico funciona como uma espécie de **“separador e identificador” do código-fonte**. Ele transforma um texto escrito pelo programador em informações estruturadas, facilitando as próximas etapas do processo de compilação.

