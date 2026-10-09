/*
 * util.h — Utilidades gerais compartilhadas por todos os módulos.
 */
#ifndef UTIL_H
#define UTIL_H

/* No Windows (MinGW), faz o printf aceitar %lld como no padrão C99.
 * Precisa vir antes de qualquer #include da biblioteca padrão; por isso
 * todo .c inclui util.h primeiro. */
#define __USE_MINGW_ANSI_STDIO 1

#include <stddef.h>

/* O programa é de execução curta: a memória alocada nunca é liberada
 * explicitamente; o sistema operacional a recupera ao final. */
void *xmalloc(size_t n);
void *xrealloc(void *p, size_t n);
char *xstrndup(const char *s, size_t n);

/* Equivalente às f-strings do Python: devolve uma string formatada nova. */
char *fmt(const char *formato, ...) __attribute__((format(printf, 1, 2)));

#endif
