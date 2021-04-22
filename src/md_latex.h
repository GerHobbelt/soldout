/* latex.h - LaTeX renderer and utilities */

#ifndef UPSKIRT_LATEX_H
#define UPSKIRT_LATEX_H

#include "document.h"
#include "buffer.h"
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

struct sd_latex_renderer_state {
	void *opaque;

	struct {
		int header_count;
		int current_level;
		int level_offset;
		int nesting_level;
	} toc_data;

	sd_render_flags flags;
	html_counter counter;
	localization localization;

	/* extra callbacks */
	void (*link_attributes)(sd_buffer *ob, const sd_buffer *url, const sd_renderer_data *data);
};
typedef struct sd_latex_renderer_state sd_latex_renderer_state;


/*************
 * FUNCTIONS *
 *************/

/* sd_html_smartypants: process an HTML snippet using SmartyPants for smart punctuation */
void sd_latex_smartypants(sd_buffer *ob, const uint8_t *data, size_t size);

/* sd_html_is_tag: checks if data starts with a specific tag, returns the tag type or NONE */
sd_render_tag sd_latex_is_tag(const uint8_t *data, size_t size, const char *tagname);


/* sd_html_renderer_new: allocates a regular HTML renderer */
sd_renderer *sd_latex_renderer_new(
	sd_render_flags render_flags,
	int nesting_level,
	localization local
) __attribute__ ((malloc));

/* sd_html_renderer_free: deallocate an HTML renderer */
void sd_latex_renderer_free(sd_renderer *renderer);


#ifdef __cplusplus
}
#endif

#endif /** UPSKIRT_LATEX_H **/
