/* autolink.h - versatile autolinker */

#ifndef UPSKIRT_AUTOLINK_H
#define UPSKIRT_AUTOLINK_H

#include "buffer.h"

#ifdef __cplusplus
extern "C" {
#endif


/*************
 * CONSTANTS *
 *************/

typedef enum sd_autolink_flags {
	UPSKIRT_AUTOLINK_SHORT_DOMAINS = (1 << 0)
} sd_autolink_flags;


/*************
 * FUNCTIONS *
 *************/

/* sd_autolink_is_safe: verify that a URL has a safe protocol */
int sd_autolink_is_safe(const uint8_t *data, size_t size);

/* sd_autolink__www: search for the next www link in data */
size_t sd_autolink__www(size_t *rewind_p, sd_buffer *link,
	uint8_t *data, size_t offset, size_t size, sd_autolink_flags flags);

/* sd_autolink__email: search for the next email in data */
size_t sd_autolink__email(size_t *rewind_p, sd_buffer *link,
	uint8_t *data, size_t offset, size_t size, sd_autolink_flags flags);

/* sd_autolink__url: search for the next URL in data */
size_t sd_autolink__url(size_t *rewind_p, sd_buffer *link,
	uint8_t *data, size_t offset, size_t size, sd_autolink_flags flags);


#ifdef __cplusplus
}
#endif

#endif /** UPSKIRT_AUTOLINK_H **/
