#include "html.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "escape.h"

#include "charter/src/parser.h"
#include "charter/src/renderer.h"

#if defined(_MSC_VER)
#define popen _popen
#endif

#define USE_XHTML(opt) (opt->flags & UPSKIRT_RENDER_USE_XHTML)
#define MAX_FILE_SIZE 1000000

static int
lang_head_len(const char *data) {
	char *end = strstr(data, "\n");
	if (!end) return -1;
	return (int)(end-data)+1;
}

static int
lang_at_tag(const char *data, const char *tag)
{
	char *tagx = NULL;
	size_t tagx_len = 0;
	char *ptr = NULL;
	char *end = NULL;

	end = strstr(data, "\n");
	if (!end) return -1;
	size_t tag_len = strlen(tag);
	if (tag_len == 0) return -1;

	tagx = malloc(tag_len+2);
	strcpy(tagx, tag);
	strcat(tagx, "@");
	tagx_len = strlen(tagx);

	ptr = strstr((const char *)data, tagx);
	free(tagx);
	if (!ptr || (ptr-end) >= 0) return -1;
	return (int)((ptr - data) + (tagx_len-1));
}

int
parse_at_attr(const uint8_t *data, char *val, int *len, const char *tag)
{
	if (!data || !val || !len || !tag) return -1;

	int outlen = 0;
	int inlen = *len;
	int dlen = lang_head_len((const char *)data);
	*len = 0;
	if (inlen <= 0) return -1;
	if (dlen <= 0) return -1;

	int k = 0;
	int quit = 0, has_attr = 0;
	k = lang_at_tag((const char *)data, tag);
	if (k < 0) return -1;

	for (; !quit && (k < dlen); k++) {
		unsigned char ch = data[k];
		switch (ch) {
			case '\n':
			case ' ':
			case '\t':
				quit = 1;
				break;
			case '@':
				if (has_attr) { // error
					quit = 1;
					has_attr = 0;
				}else {
					has_attr = 1;
				}
				break;
			default:
				if (has_attr) {
					if (outlen >= inlen) { // too long(max 16)
						quit = 1;
						has_attr = 0;
						break;
					}
					val[outlen++] = ch;
				}
				break;
		}
	}

	if (has_attr) {
		*len = outlen;
		val[outlen] = '\0';
		return k;
	}else {
		*len = 0;
		return -1;
	}
}

// parse format of "@w%", "@w%h", "@wxh", "line@wxh", "font@s"
int
parse_at_size(const uint8_t *data, int *out_w, int *out_h, const char *tag)
{
	if (!data || !out_w) return -1;

	int k = 0;
	int width = 0, height = 0, *pvalue = 0;
	int quit = 0, has_size = 0, has_sep = 0;
	int dlen = (int)strlen((const char *)data);

	if (tag) {
		k = lang_at_tag((const char *)data, tag);
		if (k < 0) return -1;
		dlen = lang_head_len((const char *)data);
	}

	for (; !quit && (k < dlen); k++) {
		unsigned char ch = data[k];
		switch (ch) {
			case '\n':
			case ' ':
			case '\t':
				quit = 1;
				break;
			case '@':
				if (has_size) { // error
					quit = 1;
					has_size = 0;
				}else {
					has_size = 1;
					pvalue = &width;
				}
				break;
			case '%':
				if (has_size) {
					height = -1;
				}
			case 'x':
				if (has_size) {
					if (has_sep) { // error
						quit = 1;
						has_size = 0;
					}else {
						has_sep = 1;
						pvalue = &height;
					}
				}
				break;
			default:
				if (has_size) {
					if (height < 0) height = 0;
					if (ch >= '0' && ch <= '9') {
						*pvalue = (*pvalue) * 10 + (ch - '0');
					}else{ // error
						quit = 1;
						has_size = 0;
					}
				}
				break;
		}
	}

	if (has_size) {
		if (out_w) *out_w = width;
		if (out_h) *out_h = height;
		return k;
	}else {
		return -1;
	}
}

sd_render_tag
sd_html_is_tag(const uint8_t *data, size_t size, const char *tagname)
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

static void escape_html(sd_buffer *ob, const uint8_t *source, size_t length)
{
	sd_escape_html(ob, source, length, 0);
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
	sd_html_renderer_state *state = data->opaque;

	if (!link || !link->size)
		return 0;

	UPSKIRT_BUFPUTSL(ob, "<a href=\"");
	if (type == UPSKIRT_AUTOLINK_EMAIL)
		UPSKIRT_BUFPUTSL(ob, "mailto:");
	escape_href(ob, link->data, link->size);

	if (state->link_attributes) {
		sd_buffer_putc(ob, '\"');
		state->link_attributes(ob, link, data);
		sd_buffer_putc(ob, '>');
	} else {
		UPSKIRT_BUFPUTSL(ob, "\">");
	}

	/*
	 * Pretty printing: if we get an email address as
	 * an actual URI, e.g. `mailto:foo@bar.com`, we don't
	 * want to print the `mailto:` prefix
	 */
	if (sd_buffer_prefix(link, "mailto:") == 0) {
		escape_html(ob, link->data + 7, link->size - 7);
	} else {
		escape_html(ob, link->data, link->size);
	}

	UPSKIRT_BUFPUTSL(ob, "</a>");

	return 1;
}

static void
rndr_blockcode(sd_buffer *ob, const sd_buffer *text, const sd_buffer *lang, const sd_renderer_data *data)
{
	if (ob->size) sd_buffer_putc(ob, '\n');
	sd_html_renderer_state *state = data->opaque;
	if (lang && (state->flags & UPSKIRT_RENDER_CHARTER) != 0 && sd_buffer_eqs(lang, "charter") != 0){
		if (text){

			char * copy = malloc((text->size + 1)*sizeof(char));
			memset(copy, 0, text->size+1);
			memcpy(copy, text->data, text->size);

			chart * c =  parse_chart(copy);
			char * svg = chart_to_svg(c);

			int n = strlen(svg);
			sd_buffer_printf(ob, svg, n);

			free(copy);
			chart_free(c);
			free(svg);

		}
		return;
	}
	if (lang &&  (state->flags & UPSKIRT_RENDER_GNUPLOT) != 0 && sd_buffer_eqs(lang, "gnuplot"))
	{
		if (text && text->size){
			char * copy = malloc((text->size + 1)*sizeof(char));
			memset(copy, 0, text->size+1);
			memcpy(copy, text->data, text->size);
			sd_buffer * b = sd_buffer_new(1);
			sd_buffer_printf(b, "gnuplot -e 'set term svg size 300,200;\n%s'", copy);

			FILE *p = popen((char*)b->data, "r");
			sd_buffer_free(b);
			char buffer[MAX_FILE_SIZE];
			size_t i;
			for (i = 0; i < MAX_FILE_SIZE; ++i)
			{
			    int c = getc(p);

			    if (c == EOF)
			    {
			        buffer[i] = 0x00;
			        break;
			    }

			    buffer[i] = c;
			}
			if (i)
			{
				sd_buffer_printf(ob, buffer, i);
			}
		}

		return;
	}
	if (lang && (state->flags & UPSKIRT_RENDER_MERMAID) != 0 && sd_buffer_eqs(lang, "mermaid") != 0){
		if (text){
	        UPSKIRT_BUFPUTSL(ob, "<div class=\"mermaid\">");
			sd_buffer_put(ob, text->data, text->size);
			UPSKIRT_BUFPUTSL(ob, "</div>");
		}
		return;
	}
	if (lang) {
		UPSKIRT_BUFPUTSL(ob, "<pre><code class=\"language-");
		escape_html(ob, lang->data, lang->size);
		UPSKIRT_BUFPUTSL(ob, "\">");
	} else {
		UPSKIRT_BUFPUTSL(ob, "<pre><code>");
	}

	if (text)
		escape_html(ob, text->data, text->size);

	UPSKIRT_BUFPUTSL(ob, "</code></pre>\n");
}

static void
rndr_blockquote(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	if (ob->size) sd_buffer_putc(ob, '\n');
	UPSKIRT_BUFPUTSL(ob, "<blockquote>\n");
	if (content) sd_buffer_put(ob, content->data, content->size);
	UPSKIRT_BUFPUTSL(ob, "</blockquote>\n");
}

static int
rndr_codespan(sd_buffer *ob, const sd_buffer *text, const sd_renderer_data *data)
{
	UPSKIRT_BUFPUTSL(ob, "<code>");
	if (text) escape_html(ob, text->data, text->size);
	UPSKIRT_BUFPUTSL(ob, "</code>");
	return 1;
}

static int
rndr_strikethrough(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	if (!content || !content->size)
		return 0;

	UPSKIRT_BUFPUTSL(ob, "<del>");
	sd_buffer_put(ob, content->data, content->size);
	UPSKIRT_BUFPUTSL(ob, "</del>");
	return 1;
}

static int
rndr_double_emphasis(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	if (!content || !content->size)
		return 0;

	UPSKIRT_BUFPUTSL(ob, "<strong>");
	sd_buffer_put(ob, content->data, content->size);
	UPSKIRT_BUFPUTSL(ob, "</strong>");

	return 1;
}

static int
rndr_emphasis(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	if (!content || !content->size) return 0;
	UPSKIRT_BUFPUTSL(ob, "<em>");
	if (content) sd_buffer_put(ob, content->data, content->size);
	UPSKIRT_BUFPUTSL(ob, "</em>");
	return 1;
}

static int
rndr_underline(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	if (!content || !content->size)
		return 0;

	UPSKIRT_BUFPUTSL(ob, "<u>");
	sd_buffer_put(ob, content->data, content->size);
	UPSKIRT_BUFPUTSL(ob, "</u>");

	return 1;
}

static int
rndr_highlight(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	if (!content || !content->size)
		return 0;

	UPSKIRT_BUFPUTSL(ob, "<mark>");
	sd_buffer_put(ob, content->data, content->size);
	UPSKIRT_BUFPUTSL(ob, "</mark>");

	return 1;
}

static int
rndr_quote(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	if (!content || !content->size)
		return 0;

	UPSKIRT_BUFPUTSL(ob, "<q>");
	sd_buffer_put(ob, content->data, content->size);
	UPSKIRT_BUFPUTSL(ob, "</q>");

	return 1;
}

static int
rndr_cite(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	if (!content || !content->size)
		return 0;

	UPSKIRT_BUFPUTSL(ob, "<cite>");
	sd_buffer_put(ob, content->data, content->size);
	UPSKIRT_BUFPUTSL(ob, "</cite>");

	return 1;
}

static int
rndr_linebreak(sd_buffer *ob, const sd_renderer_data *data)
{
	sd_html_renderer_state *state = data->opaque;
	sd_buffer_puts(ob, USE_XHTML(state) ? "<br/>\n" : "<br>\n");
	return 1;
}

static void
rndr_header(sd_buffer *ob, const sd_buffer *content, int level, const sd_renderer_data *data, h_counter counter, int numbering)
{
	if (ob->size)
		sd_buffer_putc(ob, '\n');

	if (level > 3) {
		sd_buffer_printf(ob, "<h%d id=\"toc_%d.%d.%d.%d\">", level+1, counter.chapter, counter.section, counter.subsection, level);
	} else if (counter.subsection) {
		sd_buffer_printf(ob, "<h%d id=\"toc_%d.%d.%d\">", level+1, counter.chapter, counter.section, counter.subsection);
	} else if (counter.section) {
		sd_buffer_printf(ob, "<h%d id=\"toc_%d.%d\">", level+1, counter.chapter, counter.section);
	} else if (counter.chapter) {
		sd_buffer_printf(ob, "<h%d id=\"toc_%d\">", level+1, counter.chapter);
	}

	if (numbering && level <= 3)
	{
		if (counter.subsection) {
			sd_buffer_printf(ob, "%d.%d.%d. ", counter.chapter, counter.section, counter.subsection);
		} else if (counter.section) {
			sd_buffer_printf(ob, "%d.%d. ", counter.chapter, counter.section);
		} else if (counter.chapter) {
			sd_buffer_printf(ob, "%d. ", counter.chapter);
		}
	}


	if (content) sd_buffer_put(ob, content->data, content->size);
	sd_buffer_printf(ob, "</h%d>\n", level+1);
}

static int
rndr_link(sd_buffer *ob, const sd_buffer *content, const sd_buffer *link, const sd_buffer *title, const sd_renderer_data *data)
{
	sd_html_renderer_state *state = data->opaque;

	UPSKIRT_BUFPUTSL(ob, "<a href=\"");

	if (link && link->size)
		escape_href(ob, link->data, link->size);

	if (title && title->size) {
		UPSKIRT_BUFPUTSL(ob, "\" title=\"");
		escape_html(ob, title->data, title->size);
	}

	if (state->link_attributes) {
		sd_buffer_putc(ob, '\"');
		state->link_attributes(ob, link, data);
		sd_buffer_putc(ob, '>');
	} else {
		UPSKIRT_BUFPUTSL(ob, "\">");
	}

	if (content && content->size) sd_buffer_put(ob, content->data, content->size);
	UPSKIRT_BUFPUTSL(ob, "</a>");
	return 1;
}

static void
rndr_list(sd_buffer *ob, const sd_buffer *content, sd_list_flags flags, const sd_renderer_data *data)
{
	if (ob->size) sd_buffer_putc(ob, '\n');
	sd_buffer_puts(ob, (flags & UPSKIRT_LIST_ORDERED ? "<ol dir=\"auto\">\n" : "<ul dir=\"auto\">\n"));
	if (content) sd_buffer_put(ob, content->data, content->size);
	sd_buffer_put(ob, (const uint8_t *)(flags & UPSKIRT_LIST_ORDERED ? "</ol>\n" : "</ul>\n"), 6);
}

static void
rndr_listitem(sd_buffer *ob, const sd_buffer *content, sd_list_flags flags, const sd_renderer_data *data)
{
	UPSKIRT_BUFPUTSL(ob, "<li>");
	if (content) {
		size_t size = content->size;
		while (size && content->data[size - 1] == '\n')
			size--;

		sd_buffer_put(ob, content->data, size);
	}
	UPSKIRT_BUFPUTSL(ob, "</li>\n");
}

static void
rndr_paragraph(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	sd_html_renderer_state *state = data->opaque;
	size_t i = 0;

	if (ob->size) sd_buffer_putc(ob, '\n');

	if (!content || !content->size)
		return;

	while (i < content->size && isspace(content->data[i])) i++;

	if (i == content->size)
		return;

	UPSKIRT_BUFPUTSL(ob, "<p>");
	if (state->flags & UPSKIRT_RENDER_HARD_WRAP) {
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
	UPSKIRT_BUFPUTSL(ob, "</p>\n");
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
	UPSKIRT_BUFPUTSL(ob, "<strong><em>");
	sd_buffer_put(ob, content->data, content->size);
	UPSKIRT_BUFPUTSL(ob, "</em></strong>");
	return 1;
}

static void
rndr_hrule(sd_buffer *ob, const sd_renderer_data *data)
{

	sd_html_renderer_state *state = data->opaque;
	if (ob->size) sd_buffer_putc(ob, '\n');
	if (data->meta->doc_class == CLASS_BEAMER) {
		sd_buffer_puts(ob, "</div></div>\n");
		if (data->meta->paper_size == B169)
			sd_buffer_puts(ob, "<div class=\"slide slide_169\"><div class=\"slide_body\">");
		else
			sd_buffer_puts(ob, "<div class=\"slide\"><div class=\"slide_body\">");
	} else
		sd_buffer_puts(ob, USE_XHTML(state) ? "<hr/>\n" : "<hr>\n");
}

static int
rndr_image(sd_buffer *ob, const sd_buffer *link, const sd_buffer *title, const sd_buffer *alt, const sd_renderer_data *data)
{
	sd_html_renderer_state *state = data->opaque;
	if (!link || !link->size) return 0;



	UPSKIRT_BUFPUTSL(ob, "<img src=\"");
	escape_href(ob, link->data, link->size);
	UPSKIRT_BUFPUTSL(ob, "\" alt=\"");

	if (alt && alt->size)
		escape_html(ob, alt->data, alt->size);

	if (title && title->size) {
		int next_pos = -1;
		int width = 0, height = 0;
		if (title->data[0] == '@' && title->size >= 4) {
			next_pos = parse_at_size(title->data, &width, &height, 0);
		}
		if (next_pos > 0) {
			if (width > 0) {
				sd_buffer_printf(ob, "\" width=\"%d", width);
			}
			if (height > 0) {
				sd_buffer_printf(ob, "\" height=\"%d", height);
			}
			if (next_pos < title->size) {
				UPSKIRT_BUFPUTSL(ob, "\" title=\"");
				escape_html(ob, title->data+next_pos, title->size-next_pos);
			}
		}else {
			UPSKIRT_BUFPUTSL(ob, "\" title=\"");
			escape_html(ob, title->data, title->size); 
		}
	}

	sd_buffer_puts(ob, USE_XHTML(state) ? "\"/>" : "\">");
	return 1;
}

static int
rndr_raw_html(sd_buffer *ob, const sd_buffer *text, const sd_renderer_data *data)
{
	sd_html_renderer_state *state = data->opaque;

	/* ESCAPE overrides SKIP_HTML. It doesn't look to see if
	 * there are any valid tags, just escapes all of them. */
	if((state->flags & UPSKIRT_RENDER_ESCAPE) != 0) {
		escape_html(ob, text->data, text->size);
		return 1;
	}

	if ((state->flags & UPSKIRT_RENDER_SKIP_HTML) != 0)
		return 1;

	sd_buffer_put(ob, text->data, text->size);
	return 1;
}

static void
rndr_table(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data, sd_table_flags *flags, int cols)
{
    if (ob->size) sd_buffer_putc(ob, '\n');
	UPSKIRT_BUFPUTSL(ob, "<table dir=\"auto\">\n");
    sd_buffer_put(ob, content->data, content->size);
    UPSKIRT_BUFPUTSL(ob, "</table>\n");
}

static void
rndr_table_header(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
    if (ob->size) sd_buffer_putc(ob, '\n');
    UPSKIRT_BUFPUTSL(ob, "<thead>\n");
    sd_buffer_put(ob, content->data, content->size);
    UPSKIRT_BUFPUTSL(ob, "</thead>\n");
}

static void
rndr_table_body(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
    if (ob->size) sd_buffer_putc(ob, '\n');
    UPSKIRT_BUFPUTSL(ob, "<tbody>\n");
    sd_buffer_put(ob, content->data, content->size);
    UPSKIRT_BUFPUTSL(ob, "</tbody>\n");
}

static void
rndr_tablerow(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	UPSKIRT_BUFPUTSL(ob, "<tr>\n");
	if (content) sd_buffer_put(ob, content->data, content->size);
	UPSKIRT_BUFPUTSL(ob, "</tr>\n");
}

static void
rndr_tablecell(sd_buffer *ob, const sd_buffer *content, sd_table_flags flags, const sd_renderer_data *data)
{
	if (flags & UPSKIRT_TABLE_HEADER) {
		UPSKIRT_BUFPUTSL(ob, "<th");
	} else {
		UPSKIRT_BUFPUTSL(ob, "<td");
	}

	switch (flags & UPSKIRT_TABLE_ALIGNMASK) {
	case UPSKIRT_TABLE_ALIGN_CENTER:
		UPSKIRT_BUFPUTSL(ob, " style=\"text-align: center\">");
		break;

	case UPSKIRT_TABLE_ALIGN_LEFT:
		UPSKIRT_BUFPUTSL(ob, " style=\"text-align: left\">");
		break;

	case UPSKIRT_TABLE_ALIGN_RIGHT:
		UPSKIRT_BUFPUTSL(ob, " style=\"text-align: right\">");
		break;

	default:
		UPSKIRT_BUFPUTSL(ob, ">");
	}

	if (content)
		sd_buffer_put(ob, content->data, content->size);

	if (flags & UPSKIRT_TABLE_HEADER) {
		UPSKIRT_BUFPUTSL(ob, "</th>\n");
	} else {
		UPSKIRT_BUFPUTSL(ob, "</td>\n");
	}
}

static int
rndr_superscript(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	if (!content || !content->size) return 0;
	UPSKIRT_BUFPUTSL(ob, "<sup>");
	sd_buffer_put(ob, content->data, content->size);
	UPSKIRT_BUFPUTSL(ob, "</sup>");
	return 1;
}

static void
rndr_normal_text(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	if (content)
		escape_html(ob, content->data, content->size);
}

static void
rndr_footnotes(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	sd_html_renderer_state *state = data->opaque;

	if (ob->size) sd_buffer_putc(ob, '\n');
	UPSKIRT_BUFPUTSL(ob, "<div class=\"footnotes\">\n");
	sd_buffer_puts(ob, USE_XHTML(state) ? "<hr/>\n" : "<hr>\n");
	UPSKIRT_BUFPUTSL(ob, "<ol>\n");

	if (content) sd_buffer_put(ob, content->data, content->size);

	UPSKIRT_BUFPUTSL(ob, "\n</ol>\n</div>\n");
}

static void
rndr_footnote_def(sd_buffer *ob, const sd_buffer *content, unsigned int num, const sd_renderer_data *data)
{
	size_t i = 0;
	int pfound = 0;

	/* insert anchor at the end of first paragraph block */
	if (content) {
		while ((i+3) < content->size) {
			if (content->data[i++] != '<') continue;
			if (content->data[i++] != '/') continue;
			if (content->data[i++] != 'p' && content->data[i] != 'P') continue;
			if (content->data[i] != '>') continue;
			i -= 3;
			pfound = 1;
			break;
		}
	}

	sd_buffer_printf(ob, "\n<li id=\"fn%d\">\n", num);
	if (pfound) {
		sd_buffer_put(ob, content->data, i);
		sd_buffer_printf(ob, "&nbsp;<a href=\"#fnref%d\" rev=\"footnote\">&#8617;</a>", num);
		sd_buffer_put(ob, content->data + i, content->size - i);
	} else if (content) {
		sd_buffer_put(ob, content->data, content->size);
	}
	UPSKIRT_BUFPUTSL(ob, "</li>\n");
}

static int
rndr_footnote_ref(sd_buffer *ob, int num, int is_crossref, const sd_renderer_data *data)
{
    if (num >= 0) {
		if (is_crossref) {
			sd_buffer_printf(ob, "<a href=\"#fn%d\" rel=\"crossref footnote\">%d</a>", num, num);
		}
		else {
			sd_buffer_printf(ob, "<sup id=\"fnref%d\"><a href=\"#fn%d\" rel=\"footnote\">%d</a></sup>", num, num, num);
		}
	} else {
		sd_buffer_printf(ob, "<sup>?</sup>");
	}
	return 1;
}

static int
rndr_math(sd_buffer *ob, const sd_buffer *text, int displaymode, const sd_renderer_data *data)
{
	sd_buffer_put(ob, (const uint8_t *)(displaymode ? "\\[" : "\\("), 2);

	escape_html(ob, text->data, text->size);
	sd_buffer_put(ob, (const uint8_t *)(displaymode ? "\\]" : "\\)"), 2);
	return 1;
}

static int
rndr_ruby(sd_buffer *ob, const sd_buffer *content, const sd_buffer *ruby, const sd_renderer_data *data)
{
	if (!content || !content->size) return 0;
	UPSKIRT_BUFPUTSL(ob, "<ruby>");
	sd_buffer_put(ob, content->data, content->size);

	if (ruby && ruby->size) {
		UPSKIRT_BUFPUTSL(ob, "<rp>(</rp><rt>");
		sd_buffer_put(ob, ruby->data, ruby->size);
		UPSKIRT_BUFPUTSL(ob, "</rt><rp>)</rp>");
	}

	UPSKIRT_BUFPUTSL(ob, "</ruby>");
	return 1;
}

static void
rndr_head(sd_buffer *ob, metadata * doc_meta, ext_definition * extension)
{
	sd_buffer_puts(ob, "<!DOCTYPE html>\n<html><head>\n<meta charset=\"UTF-8\">\n");
	if (doc_meta->title){
		sd_buffer_printf(ob, "<title>%s</title>\n", doc_meta->title);
	}
	if (doc_meta->authors)
	{
		sd_buffer_puts(ob, "<meta name=\"author\" content=\"");
		Strings * it;
		for (it = doc_meta->authors; it != NULL; it = it->next)
		{
			if (it->size == 1) {
				sd_buffer_puts(ob, it->str);
			} else {
				sd_buffer_printf(ob, "%s, ", it->str);
			}
		}
		sd_buffer_puts(ob, "\">\n");
	}
	if (doc_meta->keywords)
	{
		sd_buffer_printf(ob, "<meta name=\"keywords\" content=\"%s\">\n", doc_meta->keywords);
	}
	if (doc_meta->style)
	{
		sd_buffer_printf(ob, "<link rel=\"stylesheet\" href=\"%s\">\n", doc_meta->style);
	}
	if (extension && extension->extra_header)
	{
		sd_buffer_puts(ob, extension->extra_header);
	}

	sd_buffer_puts(ob, "</head>\n<body>\n");
}

static void
rndr_title(sd_buffer *ob, const sd_buffer *content, const metadata *data)
{
	sd_buffer_puts(ob, "<h1 class=\"title\">");
	escape_html(ob, content->data, content->size);
	sd_buffer_puts(ob, "</h1>\n");
}

static void
rndr_authors(sd_buffer *ob, Strings *authors)
{
	sd_buffer_puts(ob, "<div class=\"authors\">");
	Strings *it;
	for (it = authors; it != NULL; it = it->next){
		if (it->str){
			sd_buffer_printf(ob,"<span class=\"author\">%s</span><span class=\"and\"> </span>", it->str);
		}
	}
	sd_buffer_puts(ob, "</div>\n");
}

static void
rndr_affiliation(sd_buffer *ob, const sd_buffer *content,  const sd_renderer_data *data)
{
	sd_buffer_puts(ob, "<div class=\"affiliation\">");
	escape_html(ob, content->data, content->size);
	sd_buffer_puts(ob, "</div>\n");
}

static void
rndr_keywords(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data)
{
	sd_buffer_puts(ob, "<div class=\"keywords\">");
	sd_buffer_puts(ob, "<b>Keywords: </b>");
	escape_html(ob, content->data, content->size);
	sd_buffer_puts(ob, "</div>\n");
}

static void
rndr_begin(sd_buffer *ob, const sd_renderer_data *data)
{
	if (data->meta->doc_class == CLASS_BEAMER) {
		if (data->meta->title || data->meta->authors || data->meta->affiliation) {
			if (data->meta->paper_size == B169)
				sd_buffer_puts(ob, "<div class=\"document\">\n<div class=\"header slide slide_169\"><div class=\"slide_body\">");
			else
				sd_buffer_puts(ob, "<div class=\"document\">\n<div class=\"header slide\"><div class=\"slide_body\">");
		}
	} else
		sd_buffer_puts(ob, "<div class=\"document\">\n<div class=\"header\">");
}

static void
rndr_inner(sd_buffer *ob, const sd_renderer_data *data)
{
	if (data->meta->doc_class == CLASS_BEAMER) {
		if (data->meta->title || data->meta->authors || data->meta->affiliation) {

			sd_buffer_puts(ob, "</div></div><div class=\"inner\">");
		}
	} else
		sd_buffer_puts(ob, "</div><div class=\"inner\">");
	if (data->meta->doc_class == CLASS_BEAMER) {
		if (data->meta->paper_size == B169)
			sd_buffer_puts(ob, "\n<div class=\"slide slide_169\"><div class=\"slide_body\">");
		else
			sd_buffer_puts(ob, "\n<div class=\"slide\"><div class=\"slide_body\">");
	}
}

static void
rndr_end(sd_buffer *ob, ext_definition * extension, const sd_renderer_data *data)
{
	if (data->meta->doc_class == CLASS_BEAMER) {
		sd_buffer_puts(ob, "</div></div>\n");
	}
	sd_buffer_puts(ob, "</div>\n</div>\n");
	if (extension && extension->extra_closing)
	{
		sd_buffer_puts(ob, extension->extra_closing);
	}
	sd_buffer_puts(ob, "</body>\n</html>\n");
}

static void
rndr_pagebreak(sd_buffer *ob)
{
	sd_buffer_puts(ob, "<div class=\"pagebreak\"> </div>");
}


static void
rndr_abstract(sd_buffer *ob){
	sd_buffer_puts(ob, "<div id=\"abstract\" class=\"abstract\">\n");
	sd_buffer_puts(ob, "<h2>Abstract</h2>\n");
}

static void
rndr_close(sd_buffer *ob){
	sd_buffer_puts(ob, "\n</div>\n");
}

static int rndr_ref (sd_buffer *ob, char * id, int count)
{
	if (count < 0 )	{
		sd_buffer_printf(ob, "<a href=\"#%s\">(\?\?)</a>", id);
	}else {
		sd_buffer_printf(ob, "<a href=\"#%s\">%d</a>", id, count);
	}
	return 1;
}

static void rndr_open_equation(sd_buffer *ob, const char * ref, const sd_renderer_data *data)
{
	if (ref){
		sd_buffer_puts(ob,"<div id=\"");
		sd_buffer_puts(ob, ref);
		sd_buffer_puts(ob, "\" class=\"equation\">\n");
	}
	else {
		sd_buffer_puts(ob, "<div class=\"equation\">\n");
	}
	sd_html_renderer_state *state = data->opaque;
	state->counter.equation ++;
	sd_buffer_printf(ob, "<table class=\"eq_table\"><tr><td class=\"eq_code\">");
}

static void rndr_close_equation(sd_buffer *ob, const sd_renderer_data *data)
{
	sd_html_renderer_state *state = data->opaque;
	sd_buffer_printf(ob, "</td><td class=\"counter\">\\[(%d)\\]</td></tr></table></div>\n", state->counter.equation);
}

static void rndr_open_float(sd_buffer *ob, float_args args, const sd_renderer_data *data)
{
	if (args.id){
		sd_buffer_puts(ob,"<figure id=\"");
		sd_buffer_puts(ob, args.id);
		sd_buffer_puts(ob, "\">\n");
		return;
	}
	sd_buffer_puts(ob, "<figure>\n");
}

static void rnrd_close_float(sd_buffer *ob, float_args args, const sd_renderer_data *data)
{
	sd_html_renderer_state *state = data->opaque;

	if (args.caption){
		sd_buffer_puts(ob, "<figcaption><b>");
		switch (args.type)
		{
		case FIGURE:
			state->counter.figure++;
			sd_buffer_printf(ob,  "Figure %u.</b> ", state->counter.figure);
			break;
		case LISTING:
			state->counter.listing++;
			sd_buffer_printf(ob,  "Listing %u.</b> ", state->counter.listing);
			break;
		case TABLE:
			state->counter.table++;
			sd_buffer_printf(ob,  "Table %u.</b> ", state->counter.table);
			break;
		default:
			break;
		}
		sd_buffer_puts(ob, args.caption);
		sd_buffer_puts(ob, "</figcaption>\n");
	}
	sd_buffer_puts(ob, "</figure>\n");
}

static void
rndr_toc_entry(sd_buffer *ob, toc * tree, int * chapter, int * section, int * subsection, int numbering)
{
	if (!tree)
		return;
	if (tree->nesting == 1) {
		if ((*chapter))
			sd_buffer_puts(ob, "</ul>\n");
		(*chapter) ++;

		if ((*section))
			sd_buffer_puts(ob, "</ul>\n");

		(*section) = 0;
		(*subsection) = 0;

		sd_buffer_printf(ob, "<li><a href=\"#toc_%d\">", (*chapter));
		if (numbering)
			sd_buffer_printf(ob, "%d. ", (*chapter));
		sd_buffer_printf(ob, "%s</a></li>\n<ul dir=\"auto\">\n", tree->text);
	} else if (tree->nesting == 2)
	{
		if ((*section))
			sd_buffer_puts(ob, "</ul>\n");
		(*section) ++;
		(*subsection) = 0;
		sd_buffer_printf(ob, "<li><a href=\"#toc_%d.%d\">", (*chapter), (*section));
		if (numbering)
			sd_buffer_printf(ob, "%d.%d. ", (*chapter), (*section));
		sd_buffer_printf(ob, "%s</a></li>\n<ul dir=\"auto\">\n", tree->text);
	} else if (tree->nesting == 3)
	{
		(*subsection) ++;
		sd_buffer_printf(ob, "<li><a href=\"#toc_%d.%d.%d\">",
			                      (*chapter), (*section), (*subsection));
		if (numbering)
			sd_buffer_printf(ob, "%d.%d.%d. ",
			                      (*chapter), (*section), (*subsection));
		sd_buffer_printf(ob, "%s</a></li>\n",tree->text);
	}
	rndr_toc_entry(ob, tree->sibling, chapter, section, subsection, numbering);
}

static void
rndr_toc(sd_buffer *ob, toc * tree, int numbering)
{
	sd_buffer_puts(ob, "<div class=\"toc_container\">\n<h2 class=\"toc_header\">Table of Contents</h2>\n<ul class=\"toc_list\">\n");
	int cpt=0, sct=0, sbs=0;
	rndr_toc_entry(ob, tree, &cpt, &sct, &sbs, numbering);
	sd_buffer_puts(ob, "</ul></div>\n");
}

static void
rndr_position(sd_buffer *ob)
{
	sd_buffer_puts(ob, "<span id=\"cursor_pos\"></span>");
}


static void
toc_header(sd_buffer *ob, const sd_buffer *content, int level, const sd_renderer_data *data, h_counter none, int numbering)
{
	sd_html_renderer_state *state = data->opaque;

	if (level <= state->toc_data.nesting_level) {
		/* set the level offset if this is the first header
		 * we're parsing for the document */
		if (state->toc_data.current_level == 0)
			state->toc_data.level_offset = level - 1;

		level -= state->toc_data.level_offset;

		if (level > state->toc_data.current_level) {
			while (level > state->toc_data.current_level) {
				UPSKIRT_BUFPUTSL(ob, "<ul dir=\"auto\">\n<li>\n");
				state->toc_data.current_level++;
			}
		} else if (level < state->toc_data.current_level) {
			UPSKIRT_BUFPUTSL(ob, "</li>\n");
			while (level < state->toc_data.current_level) {
				UPSKIRT_BUFPUTSL(ob, "</ul>\n</li>\n");
				state->toc_data.current_level--;
			}
			UPSKIRT_BUFPUTSL(ob,"<li>\n");
		} else {
			UPSKIRT_BUFPUTSL(ob,"</li>\n<li>\n");
		}

		sd_buffer_printf(ob, "<a href=\"#toc_%d\">", state->toc_data.header_count++);
		if (content) sd_buffer_put(ob, content->data, content->size);
		UPSKIRT_BUFPUTSL(ob, "</a>\n");
	}
}

static int
toc_link(sd_buffer *ob, const sd_buffer *content, const sd_buffer *link, const sd_buffer *title, const sd_renderer_data *data)
{
	if (content && content->size) sd_buffer_put(ob, content->data, content->size);
	return 1;
}

static void
toc_finalize(sd_buffer *ob, int inline_render, const sd_renderer_data *data)
{
	sd_html_renderer_state *state;

	if (inline_render)
		return;

	state = data->opaque;

	while (state->toc_data.current_level > 0) {
		UPSKIRT_BUFPUTSL(ob, "</li>\n</ul>\n");
		state->toc_data.current_level--;
	}

	state->toc_data.header_count = 0;
}

sd_renderer *
sd_html_toc_renderer_new(int nesting_level, localization local)
{
	static const sd_renderer cb_default = {
		NULL,

		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,

		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		toc_header,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,

		NULL,
		rndr_codespan,
		rndr_double_emphasis,
		rndr_emphasis,
		rndr_underline,
		rndr_highlight,
		rndr_quote,
		rndr_cite,
		NULL,
		NULL,
		toc_link,
		rndr_triple_emphasis,
		rndr_strikethrough,
		rndr_superscript,
		NULL,
		NULL,
		NULL,
		rndr_ruby,
		NULL,
		NULL,

		NULL,
		rndr_normal_text,

		NULL,
		toc_finalize,
		NULL
	};

	sd_html_renderer_state *state;
	sd_renderer *renderer;

	/* Prepare the state pointer */
	state = sd_malloc(sizeof(sd_html_renderer_state));
	memset(state, 0x0, sizeof(sd_html_renderer_state));

	state->toc_data.nesting_level = nesting_level;
	state->counter.figure = 0;
	state->counter.equation = 0;
	state->counter.listing =0;
	state->counter.table = 0;
	state->localization = local;

	/* Prepare the renderer */
	renderer = sd_malloc(sizeof(sd_renderer));
	memcpy(renderer, &cb_default, sizeof(sd_renderer));

	renderer->opaque = state;
	return renderer;
}

sd_renderer *
sd_html_renderer_new(sd_render_flags render_flags, int nesting_level, localization local)
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
		rndr_position,
	};

	sd_html_renderer_state *state;
	sd_renderer *renderer;

	/* Prepare the state pointer */
	state = sd_malloc(sizeof(sd_html_renderer_state));
	memset(state, 0x0, sizeof(sd_html_renderer_state));

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

	if (render_flags & UPSKIRT_RENDER_SKIP_HTML || render_flags & UPSKIRT_RENDER_ESCAPE)
		renderer->blockhtml = NULL;

	renderer->opaque = state;
	return renderer;
}

void
sd_html_renderer_free(sd_renderer *renderer)
{
	free(renderer->opaque);
	free(renderer);
}
