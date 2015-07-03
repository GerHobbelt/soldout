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
#include <string.h>
#include <assert.h>
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

/* ------------------------------------------------------------------ */

  /* Basic buffer memory handling. */
  {
    hoedown_buffer		B;
    hoedown_buffer_init(&B, 1024, realloc, free, free);
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
  /* Growing data area size. */
  {
    hoedown_buffer		B;
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_buffer_grow(&B, 4096);
    }
    hoedown_buffer_uninit(&B);
  }

/* ------------------------------------------------------------------ */

  /* Putting raw octets in the data area. */
  {
    hoedown_buffer		B;
    static const char *		str = "Hello World!";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_buffer_put(&B, (const uint8_t*)str, strlen(str));
      assert(0 == memcmp(str, B.data, B.size));
    }
    hoedown_buffer_uninit(&B);
  }
  {
    hoedown_buffer		B;
    static const char *		str1 = "Hello ";
    static const char *		str2 = "World!";
    static const char *		str  = "Hello World!";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_buffer_put(&B, (const uint8_t*)str1, strlen(str1));
      hoedown_buffer_put(&B, (const uint8_t*)str2, strlen(str2));
      assert(0 == memcmp(str, B.data, B.size));
    }
    hoedown_buffer_uninit(&B);
  }
  /* Putting C string in the data area. */
  {
    hoedown_buffer		B;
    static const char *		str = "Hello World!";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_buffer_puts(&B, str);
      assert(0 == memcmp(str, B.data, B.size));
    }
    hoedown_buffer_uninit(&B);
  }
  {
    hoedown_buffer		B;
    static const char *		str1 = "Hello ";
    static const char *		str2 = "World!";
    static const char *		str = "Hello World!";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_buffer_puts(&B, str1);
      hoedown_buffer_puts(&B, str2);
      assert(0 == memcmp(str, B.data, B.size));
    }
    hoedown_buffer_uninit(&B);
  }
  /* Putting literal C string in the data area. */
  {
    hoedown_buffer		B;
    static const char *		str = "Hello World!";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      HOEDOWN_BUFPUTSL(&B, str);
      assert(0 == memcmp(str, B.data, B.size));
    }
    hoedown_buffer_uninit(&B);
  }
  {
    hoedown_buffer		B;
    static const char *		str  = "Hello World!";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      HOEDOWN_BUFPUTSL(&B, "Hello ");
      HOEDOWN_BUFPUTSL(&B, "World!");
      assert(0 == memcmp(str, B.data, B.size));
    }
    hoedown_buffer_uninit(&B);
  }
  /* Putting chars in the data area. */
  {
    hoedown_buffer		B;
    static const char *		str = "Hello World!";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      for (int i=0; i<strlen(str); ++i) {
	hoedown_buffer_putc(&B, str[i]);
      }
      assert(0 == memcmp(str, B.data, B.size));
    }
    hoedown_buffer_uninit(&B);
  }
  /* Putting files in the data area. */
  {
    hoedown_buffer		B;
    FILE *			M;
    char			str[32];
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    M = fopen("Makefile", "r");
    {
      hoedown_buffer_putf(&B, M);
      fseek(M, 0L, SEEK_SET);
      {
	fread(str, 32, 1, M);
	assert(0 == memcmp(str, B.data, 32));
      }
    }
    fclose(M);
    hoedown_buffer_uninit(&B);
  }

/* ------------------------------------------------------------------ */

  /* Setting raw octets in the data area. */
  {
    hoedown_buffer		B;
    static const char *		str1 = "Hello World!";
    static const char *		str2 = "Ciao Mondo!";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_buffer_put(&B, (const uint8_t*)str1, strlen(str1));
      assert(0 == memcmp(str1, B.data, B.size));
      hoedown_buffer_set(&B, (const uint8_t*)str2, strlen(str2));
      assert(0 == memcmp(str2, B.data, B.size));
    }
    hoedown_buffer_uninit(&B);
  }
  /* Setting a C string in the data area. */
  {
    hoedown_buffer		B;
    static const char *		str1 = "Hello World!";
    static const char *		str2 = "Ciao Mondo!";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_buffer_puts(&B, str1);
      assert(0 == memcmp(str1, B.data, B.size));
      hoedown_buffer_sets(&B, str2);
      assert(0 == memcmp(str2, B.data, B.size));
    }
    hoedown_buffer_uninit(&B);
  }
  /* Setting a literal C string in the data area. */
  {
    hoedown_buffer		B;
    static const char *		str1 = "Hello World!";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_buffer_puts(&B, str1);
      assert(0 == memcmp(str1, B.data, B.size));
      HOEDOWN_BUFSETSL(&B, "Ciao Mondo!");
      assert(0 == memcmp("Ciao Mondo!", B.data, B.size));
    }
    hoedown_buffer_uninit(&B);
  }

/* ------------------------------------------------------------------ */

  /* Comparing raw octets in the data area. */
  {
    hoedown_buffer		B;
    static const char *		str = "Hello World!";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_buffer_puts(&B, str);
      assert(0 != hoedown_buffer_eq(&B, (const uint8_t*)str, strlen(str)));
    }
    hoedown_buffer_uninit(&B);
  }
  {
    hoedown_buffer		B;
    static const char *		str1 = "Hello World!";
    static const char *		str2 = "Ciao Mondo!";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_buffer_puts(&B, str1);
      assert(0 == hoedown_buffer_eq(&B, (const uint8_t*)str2, strlen(str2)));
    }
    hoedown_buffer_uninit(&B);
  }
  /* Comparing C strings in the data area. */
  {
    hoedown_buffer		B;
    static const char *		str = "Hello World!";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_buffer_puts(&B, str);
      assert(0 != hoedown_buffer_eqs(&B, str));
    }
    hoedown_buffer_uninit(&B);
  }
  {
    hoedown_buffer		B;
    static const char *		str1 = "Hello World!";
    static const char *		str2 = "Ciao Mondo!";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_buffer_puts(&B, str1);
      assert(0 == hoedown_buffer_eqs(&B, str2));
    }
    hoedown_buffer_uninit(&B);
  }
  /* Comparing literal C strings in the data area. */
  {
    hoedown_buffer		B;
    static const char *		str = "Hello World!";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_buffer_puts(&B, str);
      assert(0 != HOEDOWN_BUFEQSL(&B, "Hello World!"));
    }
    hoedown_buffer_uninit(&B);
  }
  {
    hoedown_buffer		B;
    static const char *		str = "Hello World!";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_buffer_puts(&B, str);
      assert(0 == HOEDOWN_BUFEQSL(&B, "Ciao Mondo!"));
    }
    hoedown_buffer_uninit(&B);
  }
  /* Comparing prefix C strings in the data area. */
  {
    hoedown_buffer		B;
    static const char *		str1 = "Hello World!";
    static const char *		str2 = "Hello";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_buffer_puts(&B, str1);
      assert(0 == hoedown_buffer_prefix(&B, str2));
    }
    hoedown_buffer_uninit(&B);
  }
  {
    hoedown_buffer		B;
    static const char *		str1 = "Hello World!";
    static const char *		str2 = "Hello You";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_buffer_puts(&B, str1);
      assert(0 != hoedown_buffer_prefix(&B, str2));
    }
    hoedown_buffer_uninit(&B);
  }

  exit(EXIT_SUCCESS);
}

/* end of file */
