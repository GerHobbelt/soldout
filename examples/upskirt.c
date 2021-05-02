/*
 * Copyright (c) 2011, Vicent Marti
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "version.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int charter_svg_main(int argc, const char* argv[]);
extern int charter_tex_main(int argc, const char* argv[]);
extern int tiny_expr_repl_main(int argc, const char* argv[]);

extern int smartypants_main(int argc, const char* argv[]);
extern int upskirt_main(int argc, const char* argv[]);

typedef int main_f(int argc, const char* argv[]);

struct dispatch_table_rec
{
	const char* command;
	main_f* func;
};

static const struct dispatch_table_rec dispatch_table[] =
{
	{ "svg", charter_svg_main },
	{ "tex", charter_tex_main },
	{ "repl", tiny_expr_repl_main },
	{ "smarty", smartypants_main },
	{ "md", upskirt_main },
};

static void print_command_list(void)
{
	int i;
	fprintf(stderr, "\nCommands:\n");
	for (i = 0; i < sizeof(dispatch_table) / sizeof(dispatch_table[0]); i++)
	{
		fprintf(stderr, "  %s\n", dispatch_table[i].command);
	}
}

/* main • main function, interfacing STDIO with the parser */
int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("usage: upskirt command file\n");
		print_command_list();
		return EXIT_FAILURE;
    }

	const char* cmd = argv[1];
	int i;
	for (i = 0; i < sizeof(dispatch_table) / sizeof(dispatch_table[0]); i++)
	{
		const struct dispatch_table_rec* rec = &dispatch_table[i];
		if (strcmp(rec->command, cmd) == 0)
		{
			return rec->func(argc - 1, argv + 1);
		}
	}
	fprintf(stderr, "Unknown command '%s'\n", cmd);
	print_command_list();
	return EXIT_FAILURE;
}

/* vim: set filetype=c: */
