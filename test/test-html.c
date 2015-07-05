/*
  Part of: Hoedown
  Contents: test for HTML functions
  Date: Sun Jul  5, 2015

  Abstract

	Test file for version functions.

  Copyright (C) 2015 Marco Maggi <marco.maggi-ipsu@poste.it>

  See the LICENSE file.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "buffer.h"
#include "document.h"
#include "html.h"


void
print_buffer (hoedown_buffer * B)
{
  fwrite(B->data, B->size, 1, stderr);
  fprintf(stderr, "\n");
}


void
test_smartypants (void)
{
  /* No conversion. */
  {
    static const char *		in_str = "<p>Hello World!</p>";
    static const char *		ou_str = "<p>Hello World!</p>";
    hoedown_buffer		B;
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_html_smartypants(&B, (const uint8_t *)in_str, strlen(in_str));
      //print_buffer(&B);
      assert(0 != hoedown_buffer_eqs(&B, ou_str));
    }
    hoedown_buffer_uninit(&B);
  }
  /* Straight quotes into curly quotes. */
  {
    static const char *		in_str = "<p>\"Hello\" 'World'!</p>";
    static const char *		ou_str = "<p>&ldquo;Hello&rdquo; &lsquo;World&rsquo;!</p>";
    hoedown_buffer		B;
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_html_smartypants(&B, (const uint8_t *)in_str, strlen(in_str));
      //print_buffer(&B);
      assert(hoedown_buffer_eqs(&B, ou_str));
    }
    hoedown_buffer_uninit(&B);
  }
  /* Backtick quotes into curly quotes. */
  {
    static const char *		in_str = "<p>``Hello World!''</p>";
    static const char *		ou_str = "<p>&ldquo;Hello World!&rdquo;</p>";
    hoedown_buffer		B;
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_html_smartypants(&B, (const uint8_t *)in_str, strlen(in_str));
      //print_buffer(&B);
      assert(hoedown_buffer_eqs(&B, ou_str));
    }
    hoedown_buffer_uninit(&B);
  }
  /* Dashes. */
  {
    static const char *		in_str = "<p>Trouble--maker---that is what you are!</p>";
    static const char *		ou_str = "<p>Trouble&ndash;maker&mdash;that is what you are!</p>";
    hoedown_buffer		B;
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_html_smartypants(&B, (const uint8_t *)in_str, strlen(in_str));
      //print_buffer(&B);
      assert(hoedown_buffer_eqs(&B, ou_str));
    }
    hoedown_buffer_uninit(&B);
  }
  /* Ellipsis. */
  {
    static const char *		in_str = "<p>So... is this for real!?</p>";
    static const char *		ou_str = "<p>So&hellip; is this for real!?</p>";
    hoedown_buffer		B;
    hoedown_buffer_init(&B, 1024, realloc, free, free);
    {
      hoedown_html_smartypants(&B, (const uint8_t *)in_str, strlen(in_str));
      //print_buffer(&B);
      assert(hoedown_buffer_eqs(&B, ou_str));
    }
    hoedown_buffer_uninit(&B);
  }
}


int
main (int argc, const char *const argv[])
{
  test_smartypants();
  exit(EXIT_SUCCESS);
}

/* end of file */
