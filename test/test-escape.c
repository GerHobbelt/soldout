/*
  Part of: Hoedown
  Contents: test for escaping functions
  Date: Fri Jul  3, 2015

  Abstract

	Test file for escaping functions.

  Copyright (C) 2015 Marco Maggi <marco.maggi-ipsu@poste.it>

  See the LICENSE file.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "buffer.h"
#include "escape.h"

void
print_buffer (hoedown_buffer * B)
{
  fwrite(B->data, B->size, 1, stderr);
  fprintf(stderr, "\n");
}

int
main (int argc, const char *const argv[])
{
  /* Escaping URL. */
  {
    hoedown_buffer	B;
    const char *	str = "http://www.hoedown.org/index.html";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_escape_href(&B, (const uint8_t *)str, strlen(str));
      assert(0 != hoedown_buffer_eqs(&B, str));
    }
    hoedown_buffer_uninit(&B);
  }
  /* Escaping URL. */
  {
    hoedown_buffer	B;
    const char *	str = "http://www.hoedown.org/index.html?query";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_escape_href(&B, (const uint8_t *)str, strlen(str));
      assert(0 != hoedown_buffer_eqs(&B, str));
    }
    hoedown_buffer_uninit(&B);
  }
  /* Escaping URL. */
  {
    hoedown_buffer	B;
    const char *	str = "http://www.hoedown.org/index.html#fragment";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_escape_href(&B, (const uint8_t *)str, strlen(str));
      assert(0 != hoedown_buffer_eqs(&B, str));
    }
    hoedown_buffer_uninit(&B);
  }
  /* Escaping URL. */
  {
    hoedown_buffer	B;
    const char *	str1 = "Hello World!";
    const char *	str2 = "Hello%20World!";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_escape_href(&B, (const uint8_t *)str1, strlen(str1));
      //print_buffer(&B);
      assert(0 != hoedown_buffer_eqs(&B, str2));
    }
    hoedown_buffer_uninit(&B);
  }
  /* Escaping URL. */
  {
    hoedown_buffer	B;
    const char *	str1 = "100%";
    const char *	str2 = "100%";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_escape_href(&B, (const uint8_t *)str1, strlen(str1));
      //print_buffer(&B);
      assert(0 != hoedown_buffer_eqs(&B, str2));
    }
    hoedown_buffer_uninit(&B);
  }
  /* Escaping URL. */
  {
    hoedown_buffer	B;
    const char *	str1 = "Father & Son";
    const char *	str2 = "Father%20&amp;%20Son";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_escape_href(&B, (const uint8_t *)str1, strlen(str1));
      //print_buffer(&B);
      assert(0 != hoedown_buffer_eqs(&B, str2));
    }
    hoedown_buffer_uninit(&B);
  }

/* ------------------------------------------------------------------ */

  /* Escaping HTML. */
  {
    hoedown_buffer	B;
    const char *	str1 = "http://www.hoedown.org/index.html";
    const char *	str2 = "http:&#47;&#47;www.hoedown.org&#47;index.html";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_escape_html(&B, (const uint8_t *)str1, strlen(str1), 1);
      //print_buffer(&B);
      assert(0 != hoedown_buffer_eqs(&B, str2));
    }
    hoedown_buffer_uninit(&B);
  }
  /* Escaping HTML. */
  {
    hoedown_buffer	B;
    const char *	str1 = "http://www.hoedown.org/index.html?query";
    const char *	str2 = "http:&#47;&#47;www.hoedown.org&#47;index.html?query";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_escape_html(&B, (const uint8_t *)str1, strlen(str1), 1);
      assert(0 != hoedown_buffer_eqs(&B, str2));
    }
    hoedown_buffer_uninit(&B);
  }
  /* Escaping HTML. */
  {
    hoedown_buffer	B;
    const char *	str1 = "http://www.hoedown.org/index.html#fragment";
    const char *	str2 = "http:&#47;&#47;www.hoedown.org&#47;index.html#fragment";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_escape_html(&B, (const uint8_t *)str1, strlen(str1), 1);
      assert(0 != hoedown_buffer_eqs(&B, str2));
    }
    hoedown_buffer_uninit(&B);
  }
  /* Escaping HTML. */
  {
    hoedown_buffer	B;
    const char *	str1 = "Hello World!";
    const char *	str2 = "Hello World!";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_escape_html(&B, (const uint8_t *)str1, strlen(str1), 1);
      //print_buffer(&B);
      assert(0 != hoedown_buffer_eqs(&B, str2));
    }
    hoedown_buffer_uninit(&B);
  }
  /* Escaping HTML. */
  {
    hoedown_buffer	B;
    const char *	str1 = "100%";
    const char *	str2 = "100%";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_escape_html(&B, (const uint8_t *)str1, strlen(str1), 1);
      //print_buffer(&B);
      assert(0 != hoedown_buffer_eqs(&B, str2));
    }
    hoedown_buffer_uninit(&B);
  }
  /* Escaping HTML. */
  {
    hoedown_buffer	B;
    const char *	str1 = "Father & Son";
    const char *	str2 = "Father &amp; Son";
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_escape_html(&B, (const uint8_t *)str1, strlen(str1), 1);
      print_buffer(&B);
      assert(0 != hoedown_buffer_eqs(&B, str2));
    }
    hoedown_buffer_uninit(&B);
  }

  exit(EXIT_SUCCESS);
}

/* end of file */
