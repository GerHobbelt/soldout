/*
  Part of: Hoedown
  Contents: test for buffers functions
  Date: Fri Jul  3, 2015

  Abstract

	Test file for data buffers functions.

  Copyright (C) 2015 Marco Maggi <marco.maggi-ipsu@poste.it>

  See the LICENSE file.
*/

#include <stdio.h>
#include <stdlib.h>
#include "buffer.h"

void *
realloc_callback (void * ptr, size_t len)
{
  return realloc(ptr, len);
}
void
free_callback (void * ptr)
{
  free(ptr);
}

/* ------------------------------------------------------------------ */

int
main (int argc, const char *const argv[])
{
  /* Memory handler wrappers. */
  {
    void *			P;
    void *			Q;
    P = hoedown_malloc(123);
    for (int i=0; i<123; ++i) {
      ((uint8_t*)P)[i] = 0;
    }
    Q = hoedown_realloc(P, 1234);
    for (int i=0; i<1234; ++i) {
      ((uint8_t*)Q)[i] = 1;
    }
    free(Q);
  }
  /* Memory handler callbacks. */
  {
    void *			P;
    void *			Q;
    hoedown_realloc_callback	R = realloc_callback;
    hoedown_free_callback	F = free_callback;
    P = hoedown_malloc(123);
    for (int i=0; i<123; ++i) {
      ((uint8_t*)P)[i] = 0;
    }
    Q = R(P, 1234);
    for (int i=0; i<1234; ++i) {
      ((uint8_t*)Q)[i] = 1;
    }
    F(Q);
  }

  /* Basic buffer memory handling. */
  {
    hoedown_buffer		B;
    hoedown_buffer_init(&B, 1, realloc, free, free);
    hoedown_buffer_uninit(&B);
  }
  /* Basic buffer memory handling. */
  {
    hoedown_buffer *		B;
    B = hoedown_buffer_new(1);
    hoedown_buffer_free(B);
  }
  /* Basic buffer memory handling. */
  {
    hoedown_buffer *		B;
    B = hoedown_buffer_new(1);
    hoedown_buffer_reset(B);
    hoedown_buffer_free(B);
  }

  exit(EXIT_SUCCESS);
}

/* end of file */
