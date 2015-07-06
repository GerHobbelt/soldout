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


void
test_markup_type (void)
{
  {
    static const char *	str  = "Hello World!";
    assert(HOEDOWN_HTML_TAG_NONE == hoedown_html_is_tag((const uint8_t *) str, strlen(str), "p"));
  }
  {
    static const char *	str = "<p>Hello World!</p>";
    assert(HOEDOWN_HTML_TAG_OPEN == hoedown_html_is_tag((const uint8_t *) str, strlen(str), "p"));
  }
  {
    static const char *	str = "</p><p>Hello World!</p>";
    assert(HOEDOWN_HTML_TAG_CLOSE == hoedown_html_is_tag((const uint8_t *) str, strlen(str), "p"));
  }
}


static void
html_rend (const char * in_str, const char * ou_str)
{
  hoedown_buffer		B;
  hoedown_document *		D;
  hoedown_renderer *		R;

  hoedown_buffer_init(&B, 1024, realloc, free, free);
  R = hoedown_html_renderer_new(0, 0);
  D = hoedown_document_new(R, 0, 16);
  {
    hoedown_document_render(D, &B, (const uint8_t *)in_str, strlen(in_str));
    //print_buffer(&B);
    assert(hoedown_buffer_eqs(&B, ou_str));
  }
  hoedown_document_free(D);
  hoedown_html_renderer_free(R);
  hoedown_buffer_uninit(&B);
}
void
test_renderer (void)
{
  /* Simple text. */
  {
    static const char *		in_str = "Hello World!";
    static const char *		ou_str = "<p>Hello World!</p>\n";
    html_rend (in_str, ou_str);
  }
  /* Code markup. */
  {
    static const char *		in_str = "```Hello World!```";
    static const char *		ou_str = "<p><code>Hello World!</code></p>\n";
    html_rend (in_str, ou_str);
  }
  /* Emphasis markup. */
  {
    static const char *		in_str = "*Hello World!*";
    static const char *		ou_str = "<p><em>Hello World!</em></p>\n";
    html_rend (in_str, ou_str);
  }
  /* Emphasis markup. */
  {
    static const char *		in_str = "[Hello World!](http://localhost/)";
    static const char *		ou_str = "<p><a href=\"http://localhost/\">Hello World!</a></p>\n";
    html_rend (in_str, ou_str);
  }
}


static void
html_toc_rend (const char * in_str, const char * ou_str, int nesting_level)
{
  hoedown_buffer		B;
  hoedown_document *		D;
  hoedown_renderer *		R;

  hoedown_buffer_init(&B, 1024, realloc, free, free);
  R = hoedown_html_toc_renderer_new(nesting_level);
  D = hoedown_document_new(R, 0, 16);
  {
    hoedown_document_render(D, &B, (const uint8_t *)in_str, strlen(in_str));
    print_buffer(&B);
    assert(hoedown_buffer_eqs(&B, ou_str));
  }
  hoedown_document_free(D);
  hoedown_html_renderer_free(R);
  hoedown_buffer_uninit(&B);
}
void
test_toc_renderer (void)
{
  /* Simple table of contents. */
  {
    static const char *		in_str = "\
Heading1\n\
========\n\
\n\
Subheading11\n\
------------\n\
\n\
Subheading21\n\
------------\n\
\n\
Heading2\n\
========\n\
\n\
Subheading21\n\
------------\n\
\n\
Subheading22\n\
------------\n\
";
    static const char *		ou_str = "\
<ul>\n\
<li>\n\
<a href=\"#toc_0\">Heading1</a>\n\
<ul>\n\
<li>\n\
<a href=\"#toc_1\">Subheading11</a>\n\
</li>\n\
<li>\n\
<a href=\"#toc_2\">Subheading21</a>\n\
</li>\n\
</ul>\n\
</li>\n\
<li>\n\
<a href=\"#toc_3\">Heading2</a>\n\
<ul>\n\
<li>\n\
<a href=\"#toc_4\">Subheading21</a>\n\
</li>\n\
<li>\n\
<a href=\"#toc_5\">Subheading22</a>\n\
</li>\n\
</ul>\n\
</li>\n\
</ul>\n\
";
    html_toc_rend (in_str, ou_str, 3);
  }
  /* Simple table of contents. */
  {
    static const char *		in_str = "\
# Heading1\n\
\n\
## Subheading11\n\
\n\
## Subheading21\n\
\n\
# Heading2\n\
\n\
## Subheading21\n\
\n\
## Subheading22\n\
";
    static const char *		ou_str = "\
<ul>\n\
<li>\n\
<a href=\"#toc_0\">Heading1</a>\n\
<ul>\n\
<li>\n\
<a href=\"#toc_1\">Subheading11</a>\n\
</li>\n\
<li>\n\
<a href=\"#toc_2\">Subheading21</a>\n\
</li>\n\
</ul>\n\
</li>\n\
<li>\n\
<a href=\"#toc_3\">Heading2</a>\n\
<ul>\n\
<li>\n\
<a href=\"#toc_4\">Subheading21</a>\n\
</li>\n\
<li>\n\
<a href=\"#toc_5\">Subheading22</a>\n\
</li>\n\
</ul>\n\
</li>\n\
</ul>\n\
";
    html_toc_rend (in_str, ou_str, 3);
  }
  /* Simple table of contents. */
  {
    static const char *		in_str = "\
# Heading1\n\
\n\
## Subheading11\n\
\n\
## Subheading21\n\
\n\
# Heading2\n\
\n\
## Subheading21\n\
\n\
## Subheading22\n\
";
    static const char *		ou_str = "\
<ul>\n\
<li>\n\
<a href=\"#toc_0\">Heading1</a>\n\
<ul>\n\
<li>\n\
<a href=\"#toc_1\">Subheading11</a>\n\
</li>\n\
<li>\n\
<a href=\"#toc_2\">Subheading21</a>\n\
</li>\n\
</ul>\n\
</li>\n\
<li>\n\
<a href=\"#toc_3\">Heading2</a>\n\
<ul>\n\
<li>\n\
<a href=\"#toc_4\">Subheading21</a>\n\
</li>\n\
<li>\n\
<a href=\"#toc_5\">Subheading22</a>\n\
</li>\n\
</ul>\n\
</li>\n\
</ul>\n\
";
    html_toc_rend (in_str, ou_str, 3);
  }
  /* Table of contents, limited nesting level. */
  {
    static const char *		in_str = "\
# Heading1\n\
\n\
## Subheading11\n\
\n\
## Subheading21\n\
\n\
# Heading2\n\
\n\
## Subheading21\n\
\n\
## Subheading22\n\
";
    static const char *		ou_str = "\
<ul>\n\
<li>\n\
<a href=\"#toc_0\">Heading1</a>\n\
</li>\n\
<li>\n\
<a href=\"#toc_1\">Heading2</a>\n\
</li>\n\
</ul>\n\
";
    html_toc_rend (in_str, ou_str, 1);
  }
}


int
main (int argc, const char *const argv[])
{
  test_smartypants();
  test_markup_type();
  test_renderer();
  test_toc_renderer();
  exit(EXIT_SUCCESS);
}

/* end of file */
