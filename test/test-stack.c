/*
  Part of: Hoedown
  Contents: test for version functions
  Date: Fri Jul  3, 2015

  Abstract

	Test file for version functions.

  Copyright (C) 2015 Marco Maggi <marco.maggi-ipsu@poste.it>

  See the LICENSE file.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "stack.h"

int
main (int argc, const char *const argv[])
{
  /* Basic initialisation and finalisation. */
  {
    hoedown_stack          B;
    hoedown_stack_init(&B, 123);
    {
      assert(NULL == hoedown_stack_pop(&B));
    }
    hoedown_stack_uninit(&B);
  }
  /* Basic operations. */
  {
    hoedown_stack          B;
    hoedown_stack_init(&B, 123);
    {
      hoedown_stack_push(&B, (void*)123);
      assert(((void*)123) == hoedown_stack_top(&B));
    }
    hoedown_stack_uninit(&B);
  }
  {
    hoedown_stack          B;
    hoedown_stack_init(&B, 123);
    {
      hoedown_stack_push(&B, (void*)123);
      assert(((void*)123) == hoedown_stack_top(&B));
      hoedown_stack_push(&B, (void*)456);
      assert(((void*)456) == hoedown_stack_top(&B));
      hoedown_stack_push(&B, (void*)789);
      assert(((void*)789) == hoedown_stack_top(&B));
      assert(((void*)789) == hoedown_stack_pop(&B));
      assert(((void*)456) == hoedown_stack_pop(&B));
      assert(((void*)123) == hoedown_stack_pop(&B));
      assert(NULL == hoedown_stack_pop(&B));
    }
    hoedown_stack_uninit(&B);
  }
  exit(EXIT_SUCCESS);
}

/* end of file */
