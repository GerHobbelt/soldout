/* document.h - generic markdown parser */

#ifndef UPSKIRT_DOCUMENT_H
#define UPSKIRT_DOCUMENT_H

#include "buffer.h"
#include "autolink.h"
#include "utils.h"
#include "constants.h"

#ifdef __cplusplus
extern "C" {
#endif


/*************
 * CONSTANTS *
 *************/

typedef enum sd_extensions {
	/* block-level extensions */
	UPSKIRT_EXT_TABLES = (1 << 0),
	UPSKIRT_EXT_FENCED_CODE = (1 << 1),
	UPSKIRT_EXT_FOOTNOTES = (1 << 2),

	/* span-level extensions */
	UPSKIRT_EXT_AUTOLINK = (1 << 3),
	UPSKIRT_EXT_STRIKETHROUGH = (1 << 4),
	UPSKIRT_EXT_UNDERLINE = (1 << 5),
	UPSKIRT_EXT_HIGHLIGHT = (1 << 6),
	UPSKIRT_EXT_QUOTE = (1 << 7),
	UPSKIRT_EXT_SUPERSCRIPT = (1 << 8),
	UPSKIRT_EXT_MATH = (1 << 9),
	UPSKIRT_EXT_RUBY = (1 << 10),


	/* other flags */
	UPSKIRT_EXT_NO_INTRA_EMPHASIS = (1 << 11),
	UPSKIRT_EXT_SPACE_HEADERS = (1 << 12),
	UPSKIRT_EXT_MATH_EXPLICIT = (1 << 13),
	UPSKIRT_EXT_CITE = (1 << 14),

	/* experimental */
	UPSKIRT_EXT_SCI  = (1 << 15),

	/* negative flags */
	UPSKIRT_EXT_DISABLE_INDENTED_CODE = (1 << 16)
} sd_extensions;

#define UPSKIRT_EXT_BLOCK (\
	UPSKIRT_EXT_TABLES |\
	UPSKIRT_EXT_FENCED_CODE |\
	UPSKIRT_EXT_FOOTNOTES )

#define UPSKIRT_EXT_SPAN (\
	UPSKIRT_EXT_AUTOLINK |\
	UPSKIRT_EXT_STRIKETHROUGH |\
	UPSKIRT_EXT_UNDERLINE |\
	UPSKIRT_EXT_HIGHLIGHT |\
	UPSKIRT_EXT_QUOTE |\
	UPSKIRT_EXT_SUPERSCRIPT |\
	UPSKIRT_EXT_MATH |\
	UPSKIRT_EXT_RUBY)

#define UPSKIRT_EXT_FLAGS (\
	UPSKIRT_EXT_NO_INTRA_EMPHASIS |\
	UPSKIRT_EXT_SPACE_HEADERS |\
	UPSKIRT_EXT_MATH_EXPLICIT |\
	UPSKIRT_EXT_CITE)

#define UPSKIRT_EXT_NEGATIVE (\
	UPSKIRT_EXT_DISABLE_INDENTED_CODE )

typedef enum sd_list_flags {
	UPSKIRT_LIST_ORDERED = (1 << 0),
	UPSKIRT_LI_BLOCK = (1 << 1)	/* <li> containing block data */
} sd_list_flags;

typedef enum sd_table_flags {
	UPSKIRT_TABLE_ALIGN_LEFT = 1,
	UPSKIRT_TABLE_ALIGN_RIGHT = 2,
	UPSKIRT_TABLE_ALIGN_CENTER = 3,
	UPSKIRT_TABLE_ALIGNMASK = 3,
	UPSKIRT_TABLE_HEADER = 4
} sd_table_flags;

typedef enum sd_autolink_type {
	UPSKIRT_AUTOLINK_NONE,		/* used internally when it is not an autolink*/
	UPSKIRT_AUTOLINK_NORMAL,	/* normal http/http/ftp/mailto/etc link */
	UPSKIRT_AUTOLINK_EMAIL		/* e-mail link without explit mailto: */
} sd_autolink_type;



/*********
 * TYPES *
 *********/

struct sd_document;
typedef struct sd_document sd_document;

typedef struct metadata {
	char              *title;
	Strings           *authors;
	char              *keywords;
	char              *style;
	char              *affiliation;
	scidow_paper_size  paper_size;
	sd_doc_class  doc_class;
	int                font_size;
	int                numbering;
} metadata;

struct sd_renderer_data {
	void *opaque;
	metadata *meta;
};
typedef struct sd_renderer_data sd_renderer_data;


enum {
	FIGURE,
	TABLE,
	LISTING,
	EQUATION
}typedef float_type;

struct {
	char * id;
	char * caption;
	float_type type;
} typedef float_args;

struct {
	char * extra_header;
	char * extra_closing;
} typedef ext_definition;

struct {
	char * id;
	int32_t counter;
	float_type type;
	void * next;
} typedef reference;

struct
{
	uint32_t figure;
	uint32_t equation;
	uint32_t listing;
	uint32_t table;
}typedef html_counter;

struct
{
	int chapter;
	int section;
	int subsection;
}typedef h_counter;

struct
{
	int nesting;
	char * text;
	void * sibling;
}typedef toc;


/* sd_renderer - functions for rendering parsed data */
struct sd_renderer {
	/* state object */
	void *opaque;

	/* document level callbacks */
	void (*head)(sd_buffer *ob, metadata * doc_metadata, ext_definition * extensions);
	void (*title)(sd_buffer *ob, const sd_buffer *content,  const metadata *data);
	void (*authors)(sd_buffer *ob, Strings *authors);
	void (*affiliation)(sd_buffer *ob, const sd_buffer *content,  const sd_renderer_data *data);
	void (*keywords)(sd_buffer *ob, const sd_buffer *content,  const sd_renderer_data *data);
	void (*begin)(sd_buffer *ob, const sd_renderer_data *data);
	void (*inner)(sd_buffer *ob, const sd_renderer_data *data);
	void (*end)(sd_buffer *ob,  ext_definition * extensions, const sd_renderer_data *data);
	void (*pagebreak)(sd_buffer *ob);

	/* block level callbacks - NULL skips the block */
	void (*close)(sd_buffer *ob);
	void (*abstract)(sd_buffer *ob);
	void (*opn_equation)(sd_buffer *ob, const char * ref,const sd_renderer_data *data);
	void (*cls_equation)(sd_buffer *ob, const sd_renderer_data *data);
	void (*open_float)(sd_buffer *ob, float_args args, const sd_renderer_data *data);
	void (*close_float)(sd_buffer *ob, float_args args, const sd_renderer_data *data);
	void (*blockcode)(sd_buffer *ob, const sd_buffer *text, const sd_buffer *lang, const sd_renderer_data *data);
	void (*blockquote)(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data);
	void (*header)(sd_buffer *ob, const sd_buffer *content, int level, const sd_renderer_data *data, h_counter counter, int numbering);
	void (*hrule)(sd_buffer *ob, const sd_renderer_data *data);
	void (*list)(sd_buffer *ob, const sd_buffer *content, sd_list_flags flags, const sd_renderer_data *data);
	void (*listitem)(sd_buffer *ob, const sd_buffer *content, sd_list_flags flags, const sd_renderer_data *data);
	void (*paragraph)(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data);
	void (*table)(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data, sd_table_flags *flags, int columns);
	void (*table_header)(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data);
	void (*table_body)(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data);
	void (*table_row)(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data);
	void (*table_cell)(sd_buffer *ob, const sd_buffer *content, sd_table_flags flags, const sd_renderer_data *data);
	void (*footnotes)(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data);
	void (*footnote_def)(sd_buffer *ob, const sd_buffer *content, unsigned int num, const sd_renderer_data *data);
	void (*blockhtml)(sd_buffer *ob, const sd_buffer *text, const sd_renderer_data *data);
	void (*toc)(sd_buffer *ob, toc* ToC, int numbering);

	/* span level callbacks - NULL or return 0 prints the span verbatim */
	int (*autolink)(sd_buffer *ob, const sd_buffer *link, sd_autolink_type type, const sd_renderer_data *data);
	int (*codespan)(sd_buffer *ob, const sd_buffer *text, const sd_renderer_data *data);
	int (*double_emphasis)(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data);
	int (*emphasis)(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data);
	int (*underline)(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data);
	int (*highlight)(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data);
	int (*quote)(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data);
	int (*cite)(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data);
	int (*image)(sd_buffer *ob, const sd_buffer *link, const sd_buffer *title, const sd_buffer *alt, const sd_renderer_data *data);
	int (*linebreak)(sd_buffer *ob, const sd_renderer_data *data);
	int (*link)(sd_buffer *ob, const sd_buffer *content, const sd_buffer *link, const sd_buffer *title, const sd_renderer_data *data);
	int (*triple_emphasis)(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data);
	int (*strikethrough)(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data);
	int (*superscript)(sd_buffer *ob, const sd_buffer *content, const sd_renderer_data *data);
	int (*footnote_ref)(sd_buffer *ob, int num, int is_crossref, const sd_renderer_data *data);
	int (*math)(sd_buffer *ob, const sd_buffer *text, int displaymode, const sd_renderer_data *data);
	int (*eq_math)(sd_buffer *ob, const sd_buffer *text, int displaymode, const sd_renderer_data *data);
	int (*ruby)(sd_buffer *ob, const sd_buffer *content, const sd_buffer *ruby, const sd_renderer_data *data);
	int (*ref)(sd_buffer *ob, char * id, int count);
	int (*raw_html)(sd_buffer *ob, const sd_buffer *text, const sd_renderer_data *data);

	/* low level callbacks - NULL copies input directly into the output */
	void (*entity)(sd_buffer *ob, const sd_buffer *text, const sd_renderer_data *data);
	void (*normal_text)(sd_buffer *ob, const sd_buffer *text, const sd_renderer_data *data);

	/* miscellaneous callbacks */
	void (*doc_header)(sd_buffer *ob, int inline_render, const sd_renderer_data *data);
	void (*doc_footer)(sd_buffer *ob, int inline_render, const sd_renderer_data *data);
	
	/* position reference */
	void (*position)(sd_buffer *ob);
};
typedef struct sd_renderer sd_renderer;




/*************
 * FUNCTIONS *
 *************/

/* sd_document_new: allocate a new document processor instance */
sd_document *sd_document_new(
	const sd_renderer *renderer,
	sd_extensions extensions,
	ext_definition * exeternal_extensions,
    const char * base_folder,
	size_t max_nesting
) __attribute__ ((malloc));

/* sd_document_render: render regular Markdown using the document processor */
void sd_document_render(sd_document *doc, sd_buffer *ob, const uint8_t *data, size_t size, int position);

/* sd_document_render_inline: render inline Markdown using the document processor */
void sd_document_render_inline(sd_document *doc, sd_buffer *ob, const uint8_t *data, size_t size, int position);

/* sd_document_free: deallocate a document processor instance */
void sd_document_free(sd_document *doc);


metadata* document_metadata(const uint8_t *data, size_t size);

#ifdef __cplusplus
}
#endif

#endif /** UPSKIRT_DOCUMENT_H **/
