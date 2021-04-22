/* html.h - HTML renderer and utilities */

#ifndef UPSKIRT_UTILS_H
#define UPSKIRT_UTILS_H

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


/*************
 * CONSTANTS *
 *************/

typedef enum sd_render_flags {
	UPSKIRT_RENDER_SKIP_HTML  = (1 << 0),
  	UPSKIRT_RENDER_ESCAPE     = (1 << 1),
	UPSKIRT_RENDER_HARD_WRAP  = (1 << 2),
	UPSKIRT_RENDER_USE_XHTML  = (1 << 3),
	/* -- experimental flags -- */
	UPSKIRT_RENDER_MERMAID    = (1 << 4),
	UPSKIRT_RENDER_CHARTER    = (1 << 5),
	UPSKIRT_RENDER_GNUPLOT    = (1 << 6),
	UPSKIRT_RENDER_CSS        = (1 << 7),
} sd_render_flags;

typedef enum sd_render_tag {
	UPSKIRT_RENDER_TAG_NONE = 0,
	UPSKIRT_RENDER_TAG_OPEN,
	UPSKIRT_RENDER_TAG_CLOSE
} sd_render_tag;


/*********
 * TYPES *
 *********/
typedef struct {
	char* figure;
	char* listing;
	char* table;
} localization;


struct {
	char *str;
	int   size;
	void *next;
} typedef Strings;


void     free_strings (Strings *head);
Strings* add_string   (Strings *head,
                       char    *str);

void     remove_char  (char    *source,
                       char     target);
char*    clean_string (char    *string,
                       size_t  size);

#ifdef __cplusplus
}
#endif

#endif /** UPSKIRT_UTILS_H **/
