#ifndef UTIL_H

#include <stdlib.h>

void *xmalloc (size_t n);

void xfree (void *p);

char *strvcat (char **a);

char *xstrcat3 (char *a, char *b, char *c);

#endif
