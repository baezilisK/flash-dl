#include <assert.h>
#include "util.h"

void *xmalloc (size_t n) {
  void *ret = malloc (n);
  assert (ret || !n);
  return ret;
}

void xfree (void *p) {
  free (p);
}

char *xstrvcat (char **s) {
  char **u, *v, *ret;
  size_t l = 0, i = 0;
  for (u = s; *u; ++u)
    for (v = *u; *v; ++v)
      ++l;
  ret = xmalloc (l + 1);
  for (u = s; *u; ++u)
    for (v = *u; *v; ++v)
      ret[i++] = *v;
  ret[i] = 0;
  return ret;
}

char *xstrcat3 (char *a, char *b, char *c) {
  char *A[4];
    A[0] = a;
    A[1] = b;
    A[2] = c;
    A[3] = NULL;
  return xstrvcat (A);
}
