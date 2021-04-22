#include "document.h"
#include "html.h"
#include "md_latex.h"

#include "common.h"
#include "utils.h"
#include <time.h>


/* FEATURES INFO / DEFAULTS */

enum renderer_type {
	RENDERER_HTML,
	RENDERER_LATEX,
	RENDERER_HTML_TOC
};

struct extension_category_info {
	unsigned int flags;
	const char *option_name;
	const char *label;
};

struct extension_info {
	unsigned int flag;
	const char *option_name;
	const char *description;
};

struct html_flag_info {
	unsigned int flag;
	const char *option_name;
	const char *description;
};

static struct extension_category_info categories_info[] = {
	{UPSKIRT_EXT_BLOCK, "block", "Block extensions"},
	{UPSKIRT_EXT_SPAN, "span", "Span extensions"},
	{UPSKIRT_EXT_FLAGS, "flags", "Other flags"},
	{UPSKIRT_EXT_NEGATIVE, "negative", "Negative flags"},
};

static struct extension_info extensions_info[] = {
	{UPSKIRT_EXT_TABLES, "tables", "Parse PHP-Markdown style tables."},
	{UPSKIRT_EXT_FENCED_CODE, "fenced-code", "Parse fenced code blocks."},
	{UPSKIRT_EXT_FOOTNOTES, "footnotes", "Parse footnotes."},

	{UPSKIRT_EXT_AUTOLINK, "autolink", "Automatically turn safe URLs into links."},
	{UPSKIRT_EXT_STRIKETHROUGH, "strikethrough", "Parse ~~stikethrough~~ spans."},
	{UPSKIRT_EXT_UNDERLINE, "underline", "Parse _underline_ instead of emphasis."},
	{UPSKIRT_EXT_HIGHLIGHT, "highlight", "Parse ==highlight== spans."},
	{UPSKIRT_EXT_QUOTE, "quote", "Render \"quotes\" as <q>quotes</q>."},
	{UPSKIRT_EXT_SUPERSCRIPT, "superscript", "Parse super^script."},
	{UPSKIRT_EXT_MATH, "math", "Parse TeX $$math$$ syntax, Kramdown style."},

	{UPSKIRT_EXT_NO_INTRA_EMPHASIS, "disable-intra-emphasis", "Disable emphasis_between_words."},
	{UPSKIRT_EXT_SPACE_HEADERS, "space-headers", "Require a space after '#' in headers."},
	{UPSKIRT_EXT_MATH_EXPLICIT, "math-explicit", "Instead of guessing by context, parse $inline math$ and $$always block math$$ (requires --math)."},
	{UPSKIRT_EXT_SCI, "scidown", "SciDown Extension"},
	{UPSKIRT_EXT_DISABLE_INDENTED_CODE, "disable-indented-code", "Don't parse indented code blocks."},
};

static struct html_flag_info html_flags_info[] = {
	{UPSKIRT_RENDER_SKIP_HTML, "skip-html", "Strip all HTML tags."},
	{UPSKIRT_RENDER_ESCAPE, "escape", "Escape all HTML."},
	{UPSKIRT_RENDER_HARD_WRAP, "hard-wrap", "Render each linebreak as <br>."},
	{UPSKIRT_RENDER_USE_XHTML, "xhtml", "Render XHTML."},
	{UPSKIRT_RENDER_MERMAID, "mermaid", "Render mermaid diagrams."},
	{UPSKIRT_RENDER_GNUPLOT, "gnuplot", "Render gnuplot plot."},
	{UPSKIRT_RENDER_CSS, "style", "Set specified style-sheet."}
};

static const char *category_prefix = "all-";
static const char *negative_prefix = "no-";

#define DEF_IUNIT 1024
#define DEF_OUNIT 64
#define DEF_MAX_NESTING 16

/* Get local info */
static localization get_local()
{
  localization local;
  local.figure = "Figure";
  local.listing = "Listing";
  local.table = "Table";
  return local;
}


/* PRINT HELP */

static void
print_help(const char *basename)
{
	size_t i;
	size_t e;

	/* usage */
	printf("Usage: %s [OPTION]... [FILE]\n\n", basename);

	/* description */
	printf("Process the Markdown in FILE (or standard input) and render it to standard output, using the Upskirt library. "
	       "Parsing and rendering can be customized through the options below. The default is to parse pure markdown and output HTML.\n\n");

	/* main options */
	printf("Main options:\n");
	print_option('n', "max-nesting=N", "Maximum level of block nesting parsed. Default is " str(DEF_MAX_NESTING) ".");
	print_option('t', "toc-level=N", "Maximum level for headers included in the TOC. Zero disables TOC (the default).");
	print_option(  0, "html", "Render (X)HTML. The default.");
	print_option(  0, "latex", "Render as LATEX.");
	print_option(  0, "html-toc", "Render the Table of Contents in (X)HTML.");
	

	print_option('T', "time", "Show time spent in rendering.");
	print_option('i', "input-unit=N", "Reading block size. Default is " str(DEF_IUNIT) ".");
	print_option('o', "output-unit=N", "Writing block size. Default is " str(DEF_OUNIT) ".");
	print_option('h', "help", "Print this help text.");
	print_option('v', "version", "Print Upskirt version.");
	printf("\n");

	/* extensions */
	for (i = 0; i < count_of(categories_info); i++) {
		struct extension_category_info *category = categories_info+i;
		printf("%s (--%s%s):\n", category->label, category_prefix, category->option_name);
		for (e = 0; e < count_of(extensions_info); e++) {
			struct extension_info *extension = extensions_info+e;
			if (extension->flag & category->flags) {
				print_option(  0, extension->option_name, extension->description);
			}
		}
		printf("\n");
	}

	/* html-specific */
	printf("HTML-specific options:\n");
	for (i = 0; i < count_of(html_flags_info); i++) {
		struct html_flag_info *html_flag = html_flags_info+i;
		print_option(  0, html_flag->option_name, html_flag->description);
	}
	printf("\n");

	/* ending */
	printf("Flags and extensions can be negated by prepending 'no' to them, as in '--no-tables', '--no-span' or '--no-escape'. "
	       "Options are processed in order, so in case of contradictory options the last specified stands.\n\n");

	printf("When FILE is '-', read standard input. If no FILE was given, read standard input. Use '--' to signal end of option parsing. "
	       "Exit status is 0 if no errors occurred, 1 with option parsing errors, 4 with memory allocation errors or 5 with I/O errors.\n\n");
}


/* OPTION PARSING */

struct option_data {
	char *basename;
	int done;

	/* time reporting */
	int show_time;

	/* I/O */
	size_t iunit;
	size_t ounit;
	const char *filename;

	/* renderer */
	enum renderer_type renderer;
	int toc_level;
	sd_render_flags render_flags;

	/* parsing */
	sd_extensions extensions;
	size_t max_nesting;
};

static int
parse_short_option(char opt, char *next, void *opaque)
{
	struct option_data *data = opaque;
	long int num = 0;
	int isNum = next ? parseint(next, &num) : 0;

	if (opt == 'h') {
		print_help(data->basename);
		data->done = 1;
		return 0;
	}

	if (opt == 'v') {
		print_version();
		data->done = 1;
		return 0;
	}

	if (opt == 'T') {
		data->show_time = 1;
		return 1;
	}

	/* options requiring value */
	/* FIXME: add validation */

	if (opt == 'n' && isNum) {
		data->max_nesting = num;
		return 2;
	}

	if (opt == 't' && isNum) {
		data->toc_level = num;
		return 2;
	}

	if (opt == 'i' && isNum) {
		data->iunit = num;
		return 2;
	}

	if (opt == 'o' && isNum) {
		data->ounit = num;
		return 2;
	}

	fprintf(stderr, "Wrong option '-%c' found.\n", opt);
	return 0;
}

static int
parse_category_option(char *opt, struct option_data *data)
{
	size_t i;
	const char *name = strprefix(opt, category_prefix);
	if (!name) return 0;

	for (i = 0; i < count_of(categories_info); i++) {
		struct extension_category_info *category = &categories_info[i];
		if (strcmp(name, category->option_name)==0) {
			data->extensions |= category->flags;
			return 1;
		}
	}

	return 0;
}

static int
parse_flag_option(char *opt, struct option_data *data)
{
	size_t i;

	for (i = 0; i < count_of(extensions_info); i++) {
		struct extension_info *extension = &extensions_info[i];
		if (strcmp(opt, extension->option_name)==0) {
			data->extensions |= extension->flag;
			return 1;
		}
	}

	for (i = 0; i < count_of(html_flags_info); i++) {
		struct html_flag_info *html_flag = &html_flags_info[i];
		if (strcmp(opt, html_flag->option_name)==0) {
			data->render_flags |= html_flag->flag;
			return 1;
		}
	}

	return 0;
}

static int
parse_negative_option(char *opt, struct option_data *data)
{
	size_t i;
	const char *name = strprefix(opt, negative_prefix);
	if (!name) return 0;

	for (i = 0; i < count_of(categories_info); i++) {
		struct extension_category_info *category = &categories_info[i];
		if (strcmp(name, category->option_name)==0) {
			data->extensions &= ~(category->flags);
			return 1;
		}
	}

	for (i = 0; i < count_of(extensions_info); i++) {
		struct extension_info *extension = &extensions_info[i];
		if (strcmp(name, extension->option_name)==0) {
			data->extensions &= ~(extension->flag);
			return 1;
		}
	}

	for (i = 0; i < count_of(html_flags_info); i++) {
		struct html_flag_info *html_flag = &html_flags_info[i];
		if (strcmp(name, html_flag->option_name)==0) {
			data->render_flags &= ~(html_flag->flag);
			return 1;
		}
	}

	return 0;
}

static int
parse_long_option(char *opt, char *next, void *opaque)
{
	struct option_data *data = opaque;
	long int num = 0;
	int isNum = next ? parseint(next, &num) : 0;

	if (strcmp(opt, "help")==0) {
		print_help(data->basename);
		data->done = 1;
		return 0;
	}

	if (strcmp(opt, "version")==0) {
		print_version();
		data->done = 1;
		return 0;
	}

	if (strcmp(opt, "time")==0) {
		data->show_time = 1;
		return 1;
	}

	/* FIXME: validation */

	if (strcmp(opt, "max-nesting")==0 && isNum) {
		data->max_nesting = num;
		return 2;
	}
	if (strcmp(opt, "toc-level")==0 && isNum) {
		data->toc_level = num;
		return 2;
	}
	if (strcmp(opt, "input-unit")==0 && isNum) {
		data->iunit = num;
		return 2;
	}
	if (strcmp(opt, "output-unit")==0 && isNum) {
		data->ounit = num;
		return 2;
	}

	if (strcmp(opt, "html")==0) {
		data->renderer = RENDERER_HTML;
		return 1;
	}
	if (strcmp(opt, "html-toc")==0) {
		data->renderer = RENDERER_HTML_TOC;
		return 1;
	}
	if (strcmp(opt, "latex")==0) {
		data->renderer = RENDERER_LATEX;
		return 1;
	}	
	if (parse_category_option(opt, data) || parse_flag_option(opt, data) || parse_negative_option(opt, data))
		return 1;
	fprintf(stderr, "Wrong option '--%s' found.\n", opt);
	return 0;
}

static int
parse_argument(int argn, char *arg, int is_forced, void *opaque)
{
	struct option_data *data = opaque;

	if (argn == 0) {
		/* Input file */
		if (strcmp(arg, "-")!=0 || is_forced) data->filename = arg;
		return 1;
	}

	fprintf(stderr, "Too many arguments.\n");
	return 0;
}


/* MAIN LOGIC */

#if defined(MONOLITHIC)
int upskirt_main(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif
{
	struct option_data data;
	clock_t t1, t2;
	FILE *file = stdin;
	sd_buffer *ib, *ob;
	sd_renderer *renderer = NULL;
	void (*renderer_free)(sd_renderer *) = NULL;
	sd_document *document;

	/* Parse options */
	data.basename = argv[0];
	data.done = 0;
	data.show_time = 0;
	data.iunit = DEF_IUNIT;
	data.ounit = DEF_OUNIT;
	data.filename = NULL;
	data.renderer = RENDERER_HTML;
	data.toc_level = 0;
	data.render_flags = UPSKIRT_RENDER_CHARTER;
	data.extensions = UPSKIRT_EXT_BLOCK | UPSKIRT_EXT_SPAN | UPSKIRT_EXT_FLAGS;
	data.max_nesting = DEF_MAX_NESTING;

	argc = parse_options(argc, argv, parse_short_option, parse_long_option, parse_argument, &data);
	if (data.done) return 0;
	if (!argc) return 1;

	/* Open input file, if needed */
	if (data.filename) {
		file = fopen(data.filename, "r");
		if (!file) {
			fprintf(stderr, "Unable to open input file \"%s\": %s\n", data.filename, strerror(errno));
			return 5;
		}
	}

	/* Read everything */
	ib = sd_buffer_new(data.iunit);

	if (sd_buffer_putf(ib, file)) {
		fprintf(stderr, "I/O errors found while reading input.\n");
		return 5;
	}

	if (file != stdin) fclose(file);

	/* Create the renderer */
	if (data.renderer == RENDERER_HTML)
		renderer = sd_html_renderer_new(data.render_flags, data.toc_level, get_local());
	else if (data.renderer == RENDERER_HTML_TOC)
		renderer = sd_html_toc_renderer_new(data.toc_level, get_local());
	else if (data.renderer == RENDERER_LATEX)
		renderer = sd_latex_renderer_new(data.render_flags, data.toc_level, get_local());
	renderer_free = sd_html_renderer_free;

	/* Perform Markdown rendering */
	ob = sd_buffer_new(data.ounit);

	ext_definition ext = {NULL, NULL};
	if (data.renderer == RENDERER_HTML) {
		ext.extra_header = "<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/KaTeX/0.9.0-alpha2/katex.min.css\" crossorigin=\"anonymous\">\n"
							"<script src=\"https://cdnjs.cloudflare.com/ajax/libs/KaTeX/0.9.0-alpha2/katex.min.js\" crossorigin=\"anonymous\"></script>\n"
							"<script src=\"https://cdnjs.cloudflare.com/ajax/libs/KaTeX/0.9.0-alpha2/contrib/auto-render.min.js\" crossorigin=\"anonymous\"></script>\n";
		ext.extra_closing = "<script>renderMathInElement(document.body);</script>\n";
	}
	document = sd_document_new(renderer, data.extensions,&ext, NULL, data.max_nesting);

	t1 = clock();
	sd_document_render(document, ob, ib->data, ib->size, -1);
	t2 = clock();

	/* Cleanup */
	sd_buffer_free(ib);
	sd_document_free(document);
	renderer_free(renderer);

	/* Write the result to stdout */
	(void)fwrite(ob->data, 1, ob->size, stdout);
	sd_buffer_free(ob);

	if (ferror(stdout)) {
		fprintf(stderr, "I/O errors found while writing output.\n");
		return 5;
	}

	/* Show rendering time */
	if (data.show_time) {
		double elapsed;

		if (t1 == ((clock_t) -1) || t2 == ((clock_t) -1)) {
			fprintf(stderr, "Failed to get the time.\n");
			return 1;
		}

		elapsed = (double)(t2 - t1) / CLOCKS_PER_SEC;
		if (elapsed < 1)
			fprintf(stderr, "Time spent on rendering: %7.2f ms.\n", elapsed*1e3);
		else
			fprintf(stderr, "Time spent on rendering: %6.3f s.\n", elapsed);
	}

	return 0;
}
