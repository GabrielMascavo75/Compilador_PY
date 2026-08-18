import re
from sys import argv, exit

codigo = """
#include <stdio.h>
 int main() {
 int total = 2 + 3 * 4;
 // mostra o resultado
 printf(total); 
 return 0; 
 }"""

#comandos do C
comandos = {
    "int", "float", "double", "char", "void",
    "if", "else", "while", "for", "return", 
    "struct", "continue", "break", "printf",
    "scanf", "include", "import"
}

#operadores aritmeticos do C
operadores_aritmeticos = {
    "+", "-", "*", "/", "%",
    "''", "++", "--"
}

#operadores lógicos do C
operadores_logicos = {
    "&&", "||", "!"
}

#operadores relacionais do C
operadores_relacionais = {
    "==", "!=", "<", ">", "<=", ">="
}

#operador de atribuição do C
operador_atribuicao = {
    "="
}

#Pontuadores do C
pontuadores = {
    ",", ";", "(", ")", "{", "}", "[", "]",
    ":", "\t", "\n", " ", ' " '
}


#Classe dos tokens
class Token:
    def __init__(self, tipo, valor, linha, coluna):
        self.tipo = tipo
        self.valor = valor
        self.linha = linha
        self.coluna = coluna

    def __str__(self):
        return(f"Linha {self.linha},"
                f" Coluna {self.coluna}:"
                f" {self.tipo} : {self.valor}"
        )

#Classe do analisador léxico

class analisadorlexi:
    def __init__(self, codigo):

        self.codigo = codigo

        self.i = 0 #localiza a posição atual do código
        self.linha = 1
        self.coluna = 1 #controle de linha e coluna
        self.tokens = [] #lista de tokens encontrados
        self.erros = [] #lista de erros encontrados

    #ainda existe caracteres para analisar?
    def fim_do_codigo(self):
        return self.i >= len(self.codigo)

    #retorna caractere atual
    def atual(self):
        if self.fim():
            return ""
        return self.codigo[self.i]

    #retorna o próximo caractere
    def proximo(self):
        if self.i + 1 >= len(self.codigo):
            return ""
        return self.codigo[self.i + 1]

    #avanço para o próximo caractere
    def avancar(self):
        if self.fim():
            return
        if self.codigo[self.i] == "\n":
            self.linha += 1
            self.coluna = 1
        else:
            self.coluna += 1
        self.i += 1

    #vai adicionar o token
    def adicionar_token(self, tipo, valor, linha, coluna):
        token = Token(tipo, valor, linha, coluna)
        self.tokens.append(token)

     