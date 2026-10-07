CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -O2 -D_POSIX_C_SOURCE=200809L
all: minic scanner parser
minic: minic.o scanner.o parser.o arvore.o semantic.o
	$(CC) $(CFLAGS) -o $@ $^
scanner: scanner_main.o scanner.o
	$(CC) $(CFLAGS) -o $@ $^
parser: parser_main.o parser.o scanner.o arvore.o
	$(CC) $(CFLAGS) -o $@ $^
clean:
	rm -f *.o minic scanner parser
