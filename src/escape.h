/* escape.h - escape utilities */

#ifndef UPSKIRT_ESCAPE_H
#define UPSKIRT_ESCAPE_H

#include "buffer.h"

#ifdef __cplusplus
extern "C" {
#endif


/*************
 * FUNCTIONS *
 *************/

/* sd_escape_href: escape (part of) a URL inside HTML */
void sd_escape_href(sd_buffer *ob, const uint8_t *data, size_t size);

/* sd_escape_html: escape HTML */
void sd_escape_html(sd_buffer *ob, const uint8_t *data, size_t size, int secure);


#ifdef __cplusplus
}
#endif

#endif /** UPSKIRT_ESCAPE_H **/
