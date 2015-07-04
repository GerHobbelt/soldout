/*
  Part of: Hoedown
  Contents: test for autolink functions
  Date: Fri Jul  3, 2015

  Abstract

	Test file for autolink functions.

  Copyright (C) 2015 Marco Maggi <marco.maggi-ipsu@poste.it>

  See the LICENSE file.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "autolink.h"
#include "buffer.h"

void
print_buffer (hoedown_buffer * B)
{
  fwrite(B->data, B->size, 1, stderr);
  fprintf(stderr, "\n");
}

int
main (int argc, const char *const argv[])
{
  /* Test URI for safe prefix. */
  {
    const char *	str = "http://www.hoedown.org/index.html";
    assert(hoedown_autolink_is_safe((const uint8_t *)str, strlen(str)));
  }
  {
    const char *	str = "https://www.hoedown.org/index.html";
    assert(hoedown_autolink_is_safe((const uint8_t *)str, strlen(str)));
  }
  {
    const char *	str = "ftp://ftp.hoedown.org/pub/";
    assert(hoedown_autolink_is_safe((const uint8_t *)str, strlen(str)));
  }
  {
    const char *	str = "/pub/";
    assert(hoedown_autolink_is_safe((const uint8_t *)str, strlen(str)));
  }
  {
    const char *	str = "#segment";
    assert(hoedown_autolink_is_safe((const uint8_t *)str, strlen(str)));
  }
  {
    const char *	str = "mailto:kate.beckinsale@underworld.earth";
    assert(hoedown_autolink_is_safe((const uint8_t *)str, strlen(str)));
  }
  {
    const char *	str = "Hello World!";
    assert(! hoedown_autolink_is_safe((const uint8_t *)str, strlen(str)));
  }
  {
    const char *	str = "#/";
    assert(! hoedown_autolink_is_safe((const uint8_t *)str, strlen(str)));
  }

/* ------------------------------------------------------------------ */

  /* Find WWW link. */
  {
    char *		input_text   = "the URL is www.hoedown.org/index.html, got it?";
    size_t		input_len    = strlen(input_text);
    size_t		input_offset = strlen("the URL is ");

    uint8_t *		w_ptr = (uint8_t*)(input_text+input_offset);

    hoedown_buffer	link;
    size_t		offset_delta;
    size_t		rewind;

    assert('w' == input_text[input_offset]);
    assert('w' == w_ptr[0]);

    hoedown_buffer_init(&link, 1024, realloc, free, free);
    {
      offset_delta = hoedown_autolink__www(&rewind, &link, w_ptr, input_offset, input_len, 0);
      //print_buffer(&link);
      //print_buffer(&link);
      assert(strlen("www.hoedown.org/index.html") == offset_delta);
      assert(0 == rewind);
      assert(hoedown_buffer_eqs(&link, "www.hoedown.org/index.html"));
      assert(0 == strcmp((char *)(w_ptr+offset_delta), ", got it?"));
    }
    hoedown_buffer_uninit(&link);
  }

/* ------------------------------------------------------------------ */

  /* Find email link. */
  {
    char *		input_text   = "the address is kate.beckinsale@underworld.earth, got it?";
    size_t		input_len    = strlen(input_text);
    size_t		input_offset = strlen("the address is kate.beckinsale");

    uint8_t *		at_ptr = (uint8_t *)(input_text+input_offset);

    hoedown_buffer	link;
    size_t		offset_delta;
    size_t		rewind;

    assert('@' == input_text[input_offset]);
    assert('@' == at_ptr[0]);

    hoedown_buffer_init(&link, 1024, realloc, free, free);
    {
      offset_delta = hoedown_autolink__email(&rewind, &link, at_ptr, input_offset, input_len, 0);
      //print_buffer(&link);
      //fprintf(stderr, "off %lu, rewind %lu\n", offset_delta, rewind);
      assert(strlen("@underworld.earth") == offset_delta);
      assert(strlen("kate.beckinsale") == rewind);
      assert(hoedown_buffer_eqs(&link, "kate.beckinsale@underworld.earth"));
      assert(rewind + offset_delta == link.size);
      assert(0 == strcmp((char *)(at_ptr+offset_delta), ", got it?"));
    }
    hoedown_buffer_uninit(&link);
  }

/* ------------------------------------------------------------------ */

  /* Find URL link. */
  {
    char *		input_text   = "the URL is http://www.hoedown.org/index.html, got it?";
    size_t		input_len    = strlen(input_text);
    size_t		input_offset = strlen("the URL is http");

    uint8_t *		colon_ptr = (uint8_t *)(input_text+input_offset);

    hoedown_buffer	link;
    size_t		offset_delta;
    size_t		rewind;

    assert(':' == input_text[input_offset]);
    assert(':' == colon_ptr[0]);

    hoedown_buffer_init(&link, 1024, realloc, free, free);
    {
      offset_delta = hoedown_autolink__url(&rewind, &link, colon_ptr, input_offset, input_len, 0);
      //fprintf(stderr, "%lu\n", offset_delta);
      //print_buffer(&link);
      assert(strlen("://www.hoedown.org/index.html") == offset_delta);
      assert(4 == rewind);
      assert(hoedown_buffer_eqs(&link, "http://www.hoedown.org/index.html"));
      assert(rewind + offset_delta == link.size);
      assert(0 == strcmp((char *)(colon_ptr+offset_delta), ", got it?"));
    }
    hoedown_buffer_uninit(&link);
  }
  /* Find URL link. */
  {
    char *		input_text =    "the URL is ftp://ftp.hoedown.org/pub/, got it?";
    size_t		input_len    = strlen(input_text);
    size_t		input_offset = strlen("the URL is ftp");

    uint8_t *		colon_ptr = (uint8_t *)(input_text+input_offset);

    hoedown_buffer	link;
    size_t		offset_delta;
    size_t		rewind;

    assert(':' == input_text[input_offset]);
    assert(':' == colon_ptr[0]);

    hoedown_buffer_init(&link, 1024, realloc, free, free);
    {
      offset_delta = hoedown_autolink__url(&rewind, &link, colon_ptr, input_offset, input_len, 0);
      //fprintf(stderr, "%lu\n", offset_delta);
      //print_buffer(&link);
      assert(strlen("://ftp.hoedown.org/pub/") == offset_delta);
      assert(strlen("ftp") == rewind);
      assert(hoedown_buffer_eqs(&link, "ftp://ftp.hoedown.org/pub/"));
      assert(rewind + offset_delta == link.size);
      assert(0 == strcmp((char *)(colon_ptr+offset_delta), ", got it?"));
    }
    hoedown_buffer_uninit(&link);
  }


  exit(EXIT_SUCCESS);
}

/* end of file */
