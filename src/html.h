/* html.h - HTML renderer and utilities */

#ifndef HOEDOWN_HTML_H
#define HOEDOWN_HTML_H

#include "document.h"
#include "buffer.h"
#include "utils.h"


#ifdef __cplusplus
extern "C" {
#endif




struct hoedown_html_renderer_state {
	void *opaque;

	struct {
		int header_count;
		int current_level;
		int level_offset;
		int nesting_level;
	} toc_data;

	scidown_render_flags flags;
	html_counter counter;
	localization localization;

	/* extra callbacks */
	void (*link_attributes)(hoedown_buffer *ob, const hoedown_buffer *url, const hoedown_renderer_data *data);
};
typedef struct hoedown_html_renderer_state hoedown_html_renderer_state;


/*************
 * FUNCTIONS *
 *************/

int parse_at_attr(const uint8_t *data, char *val, int *len, const char *tag);
int parse_at_size(const uint8_t *data, int *out_w, int *out_h, const char *tag);

/* hoedown_html_smartypants: process an HTML snippet using SmartyPants for smart punctuation */
void hoedown_html_smartypants(hoedown_buffer *ob, const uint8_t *data, size_t size);

/* hoedown_html_is_tag: checks if data starts with a specific tag, returns the tag type or NONE */
scidown_render_tag hoedown_html_is_tag(const uint8_t *data, size_t size, const char *tagname);


/* hoedown_html_renderer_new: allocates a regular HTML renderer */
hoedown_renderer *hoedown_html_renderer_new(
	scidown_render_flags render_flags,
	int nesting_level,
	localization local
) __attribute__ ((malloc));

/* hoedown_html_toc_renderer_new: like hoedown_html_renderer_new, but the returned renderer produces the Table of Contents */
hoedown_renderer *hoedown_html_toc_renderer_new(
	int nesting_level,
	localization local
) __attribute__ ((malloc));

/* hoedown_html_renderer_free: deallocate an HTML renderer */
void hoedown_html_renderer_free(hoedown_renderer *renderer);


#ifdef __cplusplus
}
#endif

#endif /** HOEDOWN_HTML_H **/
