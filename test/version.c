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
#include "version.h"

int
main (int argc, const char *const argv[])
{
  int	major;
  int	minor;
  int	revision;
  hoedown_version(&major, &minor, &revision);
  printf("version number string: %s\n", HOEDOWN_VERSION);
  printf("version number: %d.%d.%d\n",
	 major, minor, revision);
  exit(EXIT_SUCCESS);
}

/* end of file */
