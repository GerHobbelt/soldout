#include "md_latex.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "charter/src/parser.h"
#include "charter/src/renderer.h"

#include "escape.h"

#define MAX_FILE_SIZE 1000000

sd_render_tag
sd_latex_is_tag(const uint8_t *data, size_t size, const char *tagname)
{
	size_t i;
	int closed = 0;

	if (size < 3 || data[0] != '<')
		return UPSKIRT_RENDER_TAG_NONE;

	i = 1;

	if (data[i] == '/') {
		closed = 1;
		i++;
	}

	for (; i < size; ++i, ++tagname) {
		if (*tagname == 0)
			break;

		if (data[i] != *tagname)
			return UPSKIRT_RENDER_TAG_NONE;
	}

	if (i == size)
		return UPSKIRT_RENDER_TAG_NONE;

	if (isspace(data[i]) || data[i] == '>')
		return closed ? UPSKIRT_RENDER_TAG_CLOSE : UPSKIRT_RENDER_TAG_OPEN;

	return UPSKIRT_RENDER_TAG_NONE;
}


static void escape_href(sd_buffer *ob, const uint8_t *source, size_t length)
{
	sd_escape_href(ob, source, length);
}

/********************
 * GENERIC RENDERER *
 ********************/
static int
rndr_autolink(sd_buffer *ob, const sd_buffer *link, sd_autolink_type type, const sd_renderer_data *data)
{
	sd_latex_renderer_state *state = data->opaque;

	if (!link || !link->size)
		return 0;

	UPSKIRT_BUFPUTSL(ob, "\\href{");
	if (type == UPSKIRT_AUTOLINK_EMAIL)
		UPSKIRT_BUFPUTSL(ob, "mailto:");
	escape_href(ob, link->data, link->size);

	if (state->link_attributes) {
		sd_buffer_putc(ob, '\"');
		state->link_attributes(ob, link, data);
		sd_buffer_puts(ob, "}{");
	} else {
		UPSKIRT_BUFPUTSL(ob, "\"}{");
	}

	/*
	 * Pretty printing: if we get an email address as
	 * an actual URI, e.g. `mailto:foo@bar.com`, we don't
	 * want to print the `mailto:` prefix
	 */
	if (sd_buffer_prefix(link, "mailto:") == 0) {
		sd_buffer_put(ob, link->data + 7, link->size - 7);
	} else {
		sd_buffer_put(ob, link->data, link->size);
	}

	UPSKIRT_BUFPUTSL(ob, "}");

	return 1;
}

static void
rndr_blockcode(sd_buffer *ob, const sd_buffer *text, const sd_buffer *lang, const sd_renderer_data *data)
{
	if (ob->size) sd_buffer_putc(ob, '\n');

    sd_latex_renderer_state *state = data->opaque;
	if (lang && (state->flags & UPSKIRT_RENDER_CHARTER) != 0 && sd_buffer_eqs(lang, "charter") != 0){
		if (text){

			char * copy = malloc((text->size + 1)*sizeof(char));
			memset(copy, 0, text->size+1);
			memcpy(copy, text->data, text->size);

			chart * c =  parse_chart(copy);
			char * tex = chart_to_latex(c);

			int n = strlen(tex);
			sd_buffer_printf(ob, tex, n);

			free(copy);
			chart_free(c);
			free(tex);
		}
		return;
	}

	if (lang) {
		UPSKIRT_BUFPUTSL(ob, "\\begin{lstlisting}[language=");
		sd_buffer_put(ob, lang->data, lang->size);
		UPSKIRT_BUFPUTSL(ob, "]\n");
	} else {
		UPSKIRT_BUFPUTSL(ob, "\\begin{lstlisting}\n");
	}

	if (text)
		sd_buffer_put(ob, text->data, text->size);

	UPSKIRT_BUFPUTSL(ob, "\\end{lstlisting}\n");
}

static void
rndr_blockquote(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	if (ob->size) sd_buffer_putc(ob, '\n');
	UPSKIRT_BUFPUTSL(ob, "\\begin{quote}\n");
	if (content) sd_buffer_put(ob, content->data, content->size);
	UPSKIRT_BUFPUTSL(ob, "\\end{quote}\n");
}

static int
rndr_codespan(sd_buffer *ob, const sd_buffer *text, const sd_renderer_data *data)
{
	UPSKIRT_BUFPUTSL(ob, "\\texttt{");
	if (text) sd_buffer_put(ob, text->data, text->size);
	UPSKIRT_BUFPUTSL(ob, "}");
	return 1;
}

static int
rndr_strikethrough(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	if (!content || !content->size)
		return 0;

	UPSKIRT_BUFPUTSL(ob, "\\st{");
	sd_buffer_put(ob, content->data, content->size);
	UPSKIRT_BUFPUTSL(ob, "}");
	return 1;
}

static int
rndr_double_emphasis(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	if (!content || !content->size)
		return 0;

	UPSKIRT_BUFPUTSL(ob, "{\\bf ");
	sd_buffer_put(ob, content->data, content->size);
	UPSKIRT_BUFPUTSL(ob, "}");

	return 1;
}

static int
rndr_emphasis(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	if (!content || !content->size) return 0;
	UPSKIRT_BUFPUTSL(ob, "{\\em ");
	if (content) sd_buffer_put(ob, content->data, content->size);
	UPSKIRT_BUFPUTSL(ob, "}");
	return 1;
}

static int
rndr_underline(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	if (!content || !content->size)
		return 0;

	UPSKIRT_BUFPUTSL(ob, "\\underline{");
	sd_buffer_put(ob, content->data, content->size);
	UPSKIRT_BUFPUTSL(ob, "}");

	return 1;
}

static int
rndr_highlight(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	if (!content || !content->size)
		return 0;

	UPSKIRT_BUFPUTSL(ob, "\\hl{");
	sd_buffer_put(ob, content->data, content->size);
	UPSKIRT_BUFPUTSL(ob, "}");

	return 1;
}

static int
rndr_quote(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	if (!content || !content->size)
		return 0;

	UPSKIRT_BUFPUTSL(ob, "\"");
	sd_buffer_put(ob, content->data, content->size);
	UPSKIRT_BUFPUTSL(ob, "\"");

	return 1;
}

static int
rndr_cite(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	if (!content || !content->size)
		return 0;

	/* TODO implement */
	sd_buffer_put(ob, content->data, content->size);

	return 1;
}

static int
rndr_linebreak(sd_buffer *ob, const sd_renderer_data *data)
{
	sd_buffer_puts(ob, "\n");
	return 1;
}

static void
rndr_header(sd_buffer *ob, const sd_buffer *content, int level, const sd_renderer_data *data, h_counter counter, int numbering)
{

	if (data->meta->doc_class == CLASS_BOOK || data->meta->doc_class == CLASS_REPORT) {
		level --;
	}

	if (ob->size)
		sd_buffer_putc(ob, '\n');

	if (!content)
	  return;

  	if (level == 0) {
	  sd_buffer_puts(ob, "\\chapter{");
	} else if (level == 1) {
		  if (data->meta->doc_class == CLASS_BEAMER)
	  	sd_buffer_puts(ob, "\\frametitle{");
	  else
  	    sd_buffer_puts(ob, "\\section{");
	} else if (level == 2) {
	  sd_buffer_puts(ob, "\\subsection{");
	} else if (level == 3) {
 	  sd_buffer_puts(ob, "\\subsubsection{");
	} else if (level == 4) {
	  sd_buffer_puts(ob, "\\paragraph{");
	} else if (level == 5) {
	  sd_buffer_puts(ob, "\\subparagraph{");
	}

	sd_buffer_put(ob, content->data, content->size);
    sd_buffer_puts(ob, "}\n");
}

static int
rndr_link(sd_buffer *ob, const sd_buffer *content, const sd_buffer *link, const sd_buffer *title, const sd_renderer_data *data)
{
	sd_latex_renderer_state *state = data->opaque;

	UPSKIRT_BUFPUTSL(ob, "\\href{");

	if (link && link->size)
		escape_href(ob, link->data, link->size);

	if (title && title->size) {
		UPSKIRT_BUFPUTSL(ob, "\" title=\"");
		sd_buffer_put(ob, title->data, title->size);
	}

	if (state->link_attributes) {
		sd_buffer_putc(ob, '\"');
		state->link_attributes(ob, link, data);
		sd_buffer_puts(ob, "}{");
	} else {
		UPSKIRT_BUFPUTSL(ob, "\"}{");
	}

	if (content && content->size) sd_buffer_put(ob, content->data, content->size);
	UPSKIRT_BUFPUTSL(ob, "}");
	return 1;
}

static void
rndr_list(sd_buffer *ob, const sd_buffer *content, sd_list_flags flags, const sd_renderer_data *data)
{
	if (ob->size) sd_buffer_putc(ob, '\n');
	sd_buffer_puts(ob, (flags & UPSKIRT_LIST_ORDERED ? "\\begin{enumerate}" : "\\begin{itemize}"));
	if (content) sd_buffer_put(ob, content->data, content->size);
	sd_buffer_puts(ob, (flags & UPSKIRT_LIST_ORDERED ? "\\end{enumerate}\n" : "\\end{itemize}\n"));
}

static void
rndr_listitem(sd_buffer *ob, const sd_buffer *content, sd_list_flags flags, const sd_renderer_data *data)
{
	UPSKIRT_BUFPUTSL(ob, "\n\\item ");
	if (content) {
		size_t size = content->size;
		while (size && content->data[size - 1] == '\n')
			size--;

		sd_buffer_put(ob, content->data, size);
	}
}

static void
rndr_paragraph(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	sd_latex_renderer_state *state = data->opaque;
	size_t i = 0;

	if (ob->size) sd_buffer_puts(ob, "\n");

	if (!content || !content->size)
		return;

	while (i < content->size && isspace(content->data[i])) i++;

	if (i == content->size)
		return;

	if (state->flags) {
		size_t org;
		while (i < content->size) {
			org = i;
			while (i < content->size && content->data[i] != '\n')
				i++;

			if (i > org)
				sd_buffer_put(ob, content->data + org, i - org);

			/*
			 * do not insert a line break if this newline
			 * is the last character on the paragraph
			 */
			if (i >= content->size - 1)
				break;

			rndr_linebreak(ob, data);
			i++;
		}
	} else {
		sd_buffer_put(ob, content->data + i, content->size - i);
	}
	UPSKIRT_BUFPUTSL(ob, "\n\n");
}

static void
rndr_raw_block(sd_buffer *ob, const sd_buffer *text, const sd_renderer_data *data)
{
	size_t org, sz;

	if (!text)
		return;

	/* FIXME: Do we *really* need to trim the HTML? How does that make a difference? */
	sz = text->size;
	while (sz > 0 && text->data[sz - 1] == '\n')
		sz--;

	org = 0;
	while (org < sz && text->data[org] == '\n')
		org++;

	if (org >= sz)
		return;

	if (ob->size)
		sd_buffer_putc(ob, '\n');

	sd_buffer_put(ob, text->data + org, sz - org);
	sd_buffer_putc(ob, '\n');
}

static int
rndr_triple_emphasis(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	if (!content || !content->size) return 0;
	UPSKIRT_BUFPUTSL(ob, "{\\bf{\\em ");
	sd_buffer_put(ob, content->data, content->size);
	UPSKIRT_BUFPUTSL(ob, "}}");
	return 1;
}

static void
rndr_hrule(sd_buffer *ob, const sd_renderer_data *data)
{

	if (ob->size) sd_buffer_putc(ob, '\n');
	if (data->meta->doc_class == CLASS_BEAMER) {
		sd_buffer_puts(ob, "\\end{frame}\n");
		sd_buffer_puts(ob, "\\begin{frame}\n");
	} else {
		sd_buffer_puts(ob, "\\rule{\\linewidth}{.1pt}\n");
	}
}

static int
rndr_image(sd_buffer *ob, const sd_buffer *link, const sd_buffer *title, const sd_buffer *alt, const sd_renderer_data *data)
{
	if (!link || !link->size) return 0;

	UPSKIRT_BUFPUTSL(ob, "\\includegraphics[width=\\linewidth]{");
	escape_href(ob, link->data, link->size);
	UPSKIRT_BUFPUTSL(ob, "}");

	return 1;
}

static int
rndr_raw_html(sd_buffer *ob, const sd_buffer *text, const sd_renderer_data *data)
{
	sd_latex_renderer_state *state = data->opaque;

	/* ESCAPE overrides SKIP_HTML. It doesn't look to see if
	 * there are any valid tags, just escapes all of them. */
	if((state->flags) != 0) {
		sd_buffer_put(ob, text->data, text->size);
		return 1;
	}

	if ((state->flags) != 0)
		return 1;

	sd_buffer_put(ob, text->data, text->size);
	return 1;
}

static void
rndr_table(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data, sd_table_flags *flags, int cols)
{
    if (ob->size) sd_buffer_putc(ob, '\n');
	UPSKIRT_BUFPUTSL(ob, "\\begin{tabular}{");
	int i;
	for (i = 0;i < cols;i++)
	{
		sd_buffer_puts(ob, " | ");
		switch(flags[i]  & UPSKIRT_TABLE_ALIGNMASK)
		{
		case UPSKIRT_TABLE_ALIGN_RIGHT:
			sd_buffer_putc(ob, 'r');
			break;
		case UPSKIRT_TABLE_ALIGN_CENTER:
			sd_buffer_putc(ob, 'c');
			break;
		default:
			sd_buffer_putc(ob, 'l');
		}
	}

	sd_buffer_puts(ob, " | }\n\\hline\n");
    sd_buffer_put(ob, content->data, content->size);
    UPSKIRT_BUFPUTSL(ob, "\\hline\n\\end{tabular}\n");
}

static void
rndr_table_header(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	/* TODO implement */
    if (ob->size) sd_buffer_putc(ob, '\n');
    sd_buffer_put(ob, content->data, content->size);
    UPSKIRT_BUFPUTSL(ob, "\\hline\n");
}

static void
rndr_table_body(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	/* TODO implement */
    sd_buffer_put(ob, content->data, content->size);
}

static void
rndr_tablerow(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	/* TODO implement */

	if (content) sd_buffer_put(ob, content->data, content->size);
	sd_buffer_replace_last(ob, "\\\\\n");
}

static void
rndr_tablecell(sd_buffer *ob, const sd_buffer *content, sd_table_flags flags, const sd_renderer_data *data)
{
	/* TODO implement */
	if (flags & UPSKIRT_TABLE_HEADER) {
		UPSKIRT_BUFPUTSL(ob, "{\\bf ");
	}

	if (content)
		sd_buffer_put(ob, content->data, content->size);
	if (flags & UPSKIRT_TABLE_HEADER) {
		sd_buffer_putc(ob, '}');
	}

	UPSKIRT_BUFPUTSL(ob, " & ");
}

static int
rndr_superscript(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	if (!content || !content->size) return 0;
	UPSKIRT_BUFPUTSL(ob, "\\textsuperscript{");
	sd_buffer_put(ob, content->data, content->size);
	UPSKIRT_BUFPUTSL(ob, "}");
	return 1;
}

static void
rndr_normal_text(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	if (content)
		sd_buffer_put(ob, content->data, content->size);
}

static void
rndr_footnotes(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	/*TOOD fix that*/
	/*sd_latex_renderer_state *state = data->opaque;*/
	sd_buffer_puts(ob, "\\begin{thebibliography}{00}\n");
	if (content) sd_buffer_put(ob, content->data, content->size);
	sd_buffer_puts(ob, "\\end{thebibliography}\n");
}

static void
rndr_footnote_def(sd_buffer *ob, const sd_buffer *content, unsigned int num, const sd_renderer_data *data)
{
	/*TODO fix that*/
	if (!content)
		return;

	char * tmp = malloc(content->size+1);
	tmp[content->size] = 0;
	memcpy(tmp, content->data, content->size);
	sd_buffer_printf(ob, "\\bibitem{fnref:%d}%s\n", num, tmp);
	free(tmp);
}

static int
rndr_footnote_ref(sd_buffer *ob, int num, int is_crossref, const sd_renderer_data *data)
{
	if (num > 0)
		sd_buffer_printf(ob, "\\cite{fnref:%d}", num);

	return 1;
}

static int
rndr_math(sd_buffer *ob, const sd_buffer *text, int displaymode, const sd_renderer_data *data)
{
	sd_buffer_put(ob, (const uint8_t *)(displaymode ? "\\[" : "\\("), 2);

	sd_buffer_put(ob, text->data, text->size);
	sd_buffer_put(ob, (const uint8_t *)(displaymode ? "\\]" : "\\)"), 2);
	return 1;
}

static int
rndr_eq_math(sd_buffer *ob, const sd_buffer *text, int displaymode, const sd_renderer_data *data)
{
	sd_buffer_put(ob, text->data+1, text->size-1);
	return 1;
}

static int
rndr_ruby(sd_buffer *ob, const sd_buffer *content, const sd_buffer *ruby, const sd_renderer_data *data)
{
	if (!content || !content->size) return 0;

	/* TODO implement */
	sd_buffer_put(ob, content->data, content->size);
	return 1;
}

static void
rndr_head(sd_buffer *ob, metadata * doc_meta, ext_definition * extension)
{
	sd_buffer_printf(ob, "\\documentclass[%s, %dpt]{%s}\n",
	                      paper_to_latex(doc_meta->paper_size),
	                      doc_meta->font_size,
	                      class_to_string(doc_meta->doc_class));
	sd_buffer_puts(ob, "\\usepackage[utf8]{inputenc}\n"
		                    "\\usepackage{cite}\n"
		                    "\\usepackage{amsmath,amssymb,amsfonts}\n"
		                    "\\usepackage{algorithmic}\n"
		                    "\\usepackage{float}\n"
		                    "\\usepackage{hyperref}\n"
		                    "\\usepackage{graphicx}\n"
		                    "\\usepackage{textcomp}\n"
		                    "\\usepackage{listings}\n"
		                    "\\usepackage{epsfig}\n"
		                    "\\usepackage{tikz}\n"
		                    "\\usepackage{pgfplots}\n\n"
		                    "\\pgfplotsset{compat=1.15}\n\n"
		                    "\\providecommand{\\keywords}[1]{{\\bf{\\em Index terms---}} #1}\n"
							"\\newfloat{program}{thp}{lop}\n\\floatname{program}{Listing}\n"
						);

	if (doc_meta->title){
		sd_buffer_printf(ob, "\\title{%s}\n\\date{}\n", doc_meta->title);
	}

	if (doc_meta->authors)
	{
		sd_buffer_puts(ob, "\\author{");
		Strings* it;
		for (it = doc_meta->authors; it != NULL; it = it->next){
			if (it->size == 1) {
				sd_buffer_puts(ob, it->str);
			} else {
				sd_buffer_printf(ob, "%s \\and ", it->str);
			}
		}
		sd_buffer_puts(ob, "}\n");
	}

	if (extension && extension->extra_header)
	{
		sd_buffer_puts(ob, extension->extra_header);
	}

	sd_buffer_puts(ob,"\n\\begin{document}\n");
}

static void
rndr_title(sd_buffer *ob, const sd_buffer *content, const metadata *data)
{
	if (data->doc_class == CLASS_BEAMER) {
		sd_buffer_puts(ob, "\\frame{\\titlepage}");
	} else {
		sd_buffer_puts(ob, "\\maketitle");
	}
}

static void
rndr_authors(sd_buffer *ob, Strings *authors)
{
}

static void
rndr_affiliation(sd_buffer *ob, const sd_buffer *content,  const sd_renderer_data *data)
{

}

static void
rndr_keywords(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	sd_buffer_puts(ob, "\\keywords{");
	sd_buffer_put(ob, content->data, content->size);
	sd_buffer_puts(ob, "}\n");
}

static void
rndr_begin(sd_buffer *ob, const sd_renderer_data *data)
{

}

static void
rndr_inner(sd_buffer *ob, const sd_renderer_data *data)
{
	if (data->meta->doc_class == CLASS_BEAMER)
		sd_buffer_puts(ob, "\\begin{frame}\n");
}

static void
rndr_end(sd_buffer *ob, ext_definition * extension, const sd_renderer_data *data)
{
	if (ob->size) sd_buffer_putc(ob, '\n');

	if (data->meta->doc_class == CLASS_BEAMER) {
		sd_buffer_puts(ob, "\\end{frame}\n");
	}

	if (extension && extension->extra_closing)
	{
		sd_buffer_puts(ob, extension->extra_closing);
	}
	sd_buffer_puts(ob, "\\end{document}\n");
}

static void
rndr_pagebreak(sd_buffer *ob)
{
	sd_buffer_puts(ob, "\\newpage\n");
}


static void
rndr_abstract(sd_buffer *ob){
	sd_buffer_puts(ob, "\\begin{abstract}\n");
}

static void
rndr_close(sd_buffer *ob){
	sd_buffer_puts(ob, "\\end{abstract}");
}

static int rndr_ref (sd_buffer *ob, char * id, int count)
{
	sd_buffer_printf(ob, "(\\ref{%s})", id);
	return 1;
}

static void rndr_open_equation(sd_buffer *ob, const char * ref, const sd_renderer_data *data)
{
    sd_buffer_puts(ob,"\\begin{equation}\n");
	if (ref){
		sd_buffer_puts(ob,"\\label{");
		sd_buffer_puts(ob, ref);
		sd_buffer_puts(ob, "}\n");
	}
}

static void rndr_close_equation(sd_buffer *ob, const sd_renderer_data *data)
{
	/* sd_latex_renderer_state *state = data->opaque; */
	sd_buffer_puts(ob, "\n\\end{equation}");
}

static void rndr_open_float(sd_buffer *ob, float_args args, const sd_renderer_data *data)
{
	switch (args.type)
	{
	case FIGURE:
		sd_buffer_puts(ob,  "\\begin{figure}[ht!]\n");
		break;
	case LISTING:
		/**TODO make it better**/
		sd_buffer_puts(ob,  "\\begin{program}\n");
		break;
	case TABLE:

		sd_buffer_puts(ob,  "\\begin{table}\n");
		break;
	default:
		break;
	}
	sd_buffer_puts(ob, "\\begin{center}\n");
}

static void rnrd_close_float(sd_buffer *ob, float_args args, const sd_renderer_data *data)
{
	if (args.caption){
		sd_buffer_puts(ob, "\\caption{");
		sd_buffer_puts(ob, args.caption);
		sd_buffer_puts(ob, "}\n");
	}
	if (args.id)
	{
		sd_buffer_puts(ob, "\\label{");
		sd_buffer_puts(ob, args.id);
		sd_buffer_puts(ob, "}\n");
	}

	sd_buffer_puts(ob,  "\\end{center}\n");
	switch (args.type)
	{
	case FIGURE:
		sd_buffer_puts(ob,  "\\end{figure}\n");
		break;
	case LISTING:
		/**TODO make it better**/
		sd_buffer_puts(ob,  "\\end{program}\n");
		break;
	case TABLE:

		sd_buffer_puts(ob,  "\\end{table}\n");
		break;
	default:
		break;
	}
}


static void
rndr_toc(sd_buffer *ob, toc * tree, int numbering)
{
	sd_buffer_puts(ob, "\\tableofcontents");
}

sd_renderer *
sd_latex_renderer_new(sd_render_flags render_flags, int nesting_level, localization local)
{
	static const sd_renderer cb_default = {
		NULL,

		rndr_head,
		rndr_title,
		rndr_authors,
		rndr_affiliation,
		rndr_keywords,
		rndr_begin,
		rndr_inner,
		rndr_end,
		rndr_pagebreak,

		rndr_close,
		rndr_abstract,
		rndr_open_equation,
		rndr_close_equation,
		rndr_open_float,
		rnrd_close_float,
		rndr_blockcode,
		rndr_blockquote,
		rndr_header,
		rndr_hrule,
		rndr_list,
		rndr_listitem,
		rndr_paragraph,
		rndr_table,
		rndr_table_header,
		rndr_table_body,
		rndr_tablerow,
		rndr_tablecell,
		rndr_footnotes,
		rndr_footnote_def,
		rndr_raw_block, /* html */
		rndr_toc,

		rndr_autolink,
		rndr_codespan,
		rndr_double_emphasis,
		rndr_emphasis,
		rndr_underline,
		rndr_highlight,
		rndr_quote,
		rndr_cite,
		rndr_image,
		rndr_linebreak,
		rndr_link,
		rndr_triple_emphasis,
		rndr_strikethrough,
		rndr_superscript,
		rndr_footnote_ref,
		rndr_math,
		rndr_math, /* eq_math */
		rndr_ruby,
		rndr_ref,
		rndr_raw_html,

		NULL,
		rndr_normal_text,

		NULL,
		NULL,
		NULL,
	};

	sd_latex_renderer_state *state;
	sd_renderer *renderer;

	/* Prepare the state pointer */
	state = sd_malloc(sizeof(sd_latex_renderer_state));
	memset(state, 0x0, sizeof(sd_latex_renderer_state));

	state->flags = render_flags;
	state->counter.figure = 0;
	state->counter.equation = 0;
	state->counter.listing = 0;
	state->counter.table = 0;

	state->localization = local;

	state->toc_data.nesting_level = nesting_level;

	/* Prepare the renderer */
	renderer = sd_malloc(sizeof(sd_renderer));
	memcpy(renderer, &cb_default, sizeof(sd_renderer));

	renderer->opaque = state;
	return renderer;
}

void
sd_latex_renderer_free(sd_renderer *renderer)
{
	free(renderer->opaque);
	free(renderer);
}
