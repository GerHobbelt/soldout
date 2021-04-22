/* buffer.h - simple, fast buffers */

#ifndef UPSKIRT_BUFFER_H
#define UPSKIRT_BUFFER_H

#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)
#define __attribute__(x)
#define inline __inline
#define __builtin_expect(x,n) x
#endif


/*********
 * TYPES *
 *********/

typedef void *(*sd_realloc_callback)(void *, size_t);
typedef void (*sd_free_callback)(void *);

struct sd_buffer {
	uint8_t *data;	/* actual character data */
	size_t size;	/* size of the string */
	size_t asize;	/* allocated size (0 = volatile buffer) */
	size_t unit;	/* reallocation unit size (0 = read-only buffer) */

	sd_realloc_callback data_realloc;
	sd_free_callback data_free;
	sd_free_callback buffer_free;
};

typedef struct sd_buffer sd_buffer;


/*************
 * FUNCTIONS *
 *************/

/* allocation wrappers */
void *sd_malloc(size_t size) __attribute__ ((malloc));
void *sd_calloc(size_t nmemb, size_t size) __attribute__ ((malloc));
void *sd_realloc(void *ptr, size_t size) __attribute__ ((malloc));

/* sd_buffer_init: initialize a buffer with custom allocators */
void sd_buffer_init(
	sd_buffer *buffer,
	size_t unit,
	sd_realloc_callback data_realloc,
	sd_free_callback data_free,
	sd_free_callback buffer_free
);

/* sd_buffer_uninit: uninitialize an existing buffer */
void sd_buffer_uninit(sd_buffer *buf);

/* sd_buffer_new: allocate a new buffer */
sd_buffer *sd_buffer_new(size_t unit) __attribute__ ((malloc));

/* sd_buffer_reset: free internal data of the buffer */
void sd_buffer_reset(sd_buffer *buf);

/* sd_buffer_grow: increase the allocated size to the given value */
void sd_buffer_grow(sd_buffer *buf, size_t neosz);

/* sd_buffer_put: append raw data to a buffer */
void sd_buffer_put(sd_buffer *buf, const uint8_t *data, size_t size);

/* sd_buffer_puts: append a NUL-terminated string to a buffer */
void sd_buffer_puts(sd_buffer *buf, const char *str);

/* sd_buffer_putc: append a single char to a buffer */
void sd_buffer_putc(sd_buffer *buf, uint8_t c);

/* sd_buffer_putf: read from a file and append to a buffer, until EOF or error */
int sd_buffer_putf(sd_buffer *buf, FILE* file);

/* sd_buffer_set: replace the buffer's contents with raw data */
void sd_buffer_set(sd_buffer *buf, const uint8_t *data, size_t size);

/* sd_buffer_sets: replace the buffer's contents with a NUL-terminated string */
void sd_buffer_sets(sd_buffer *buf, const char *str);

/* sd_buffer_eq: compare a buffer's data with other data for equality */
int sd_buffer_eq(const sd_buffer *buf, const uint8_t *data, size_t size);

/* sd_buffer_eq: compare a buffer's data with NUL-terminated string for equality */
int sd_buffer_eqs(const sd_buffer *buf, const char *str);

/* sd_buffer_prefix: compare the beginning of a buffer with a string */
int sd_buffer_prefix(const sd_buffer *buf, const char *prefix);

/* sd_buffer_slurp: remove a given number of bytes from the head of the buffer */
void sd_buffer_slurp(sd_buffer *buf, size_t size);

/* sd_buffer_cstr: NUL-termination of the string array (making a C-string) */
const char *sd_buffer_cstr(sd_buffer *buf);

/* sd_buffer_printf: formatted printing to a buffer */
void sd_buffer_printf(sd_buffer *buf, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));

/* sd_buffer_put_utf8: put a Unicode character encoded as UTF-8 */
void sd_buffer_put_utf8(sd_buffer *buf, unsigned int codepoint);

/* sd_buffer_replace_last: replace last part of the buffer with new string */
void sd_buffer_replace_last(sd_buffer *buf, const char * str);

/* sd_buffer_free: free the buffer */
void sd_buffer_free(sd_buffer *buf);


/* UPSKIRT_BUFPUTSL: optimized sd_buffer_puts of a string literal */
#define UPSKIRT_BUFPUTSL(output, literal) \
	sd_buffer_put(output, (const uint8_t *)literal, sizeof(literal) - 1)

/* UPSKIRT_BUFSETSL: optimized sd_buffer_sets of a string literal */
#define UPSKIRT_BUFSETSL(output, literal) \
	sd_buffer_set(output, (const uint8_t *)literal, sizeof(literal) - 1)

/* UPSKIRT_BUFEQSL: optimized sd_buffer_eqs of a string literal */
#define UPSKIRT_BUFEQSL(output, literal) \
	sd_buffer_eq(output, (const uint8_t *)literal, sizeof(literal) - 1)


#ifdef __cplusplus
}
#endif

#endif /** UPSKIRT_BUFFER_H **/
