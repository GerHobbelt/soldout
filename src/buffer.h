/*
 * Copyright (c) 2008, Natacha Porté
 * Copyright (c) 2011, Vicent Martí
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef BUFFER_H__
#define BUFFER_H__

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if _MSC_VER
#include "win32.h"
#else
#define DLLEXPORT
#endif

#if defined(_MSC_VER)
#define __attribute__(x)
#define inline
#endif

typedef enum {
	BUF_OK = 0,
	BUF_ENOMEM = -1,
} buferror_t;

typedef void *(*sd_malloc_cb)(size_t);
typedef void *(*sd_realloc_cb)(void *, size_t);
typedef void (*sd_free_cb)(void *);

/* struct buf: character array buffer */
struct buf {
	uint8_t *data;		/* actual character data */
	size_t size;	/* size of the string */
	size_t asize;	/* allocated size (0 = volatile buffer) */
	size_t unit;	/* reallocation unit size (0 = read-only buffer) */
	sd_realloc_cb realloc;
	sd_free_cb free;
};

/* CONST_BUF: global buffer from a string litteral */
#define BUF_STATIC(string) \
	{ (uint8_t *)string, sizeof string -1, sizeof string, 0, 0 }

/* VOLATILE_BUF: macro for creating a volatile buffer on the stack */
#define BUF_VOLATILE(strname) \
	{ (uint8_t *)strname, strlen(strname), 0, 0, 0 }

/* BUFPUTSL: optimized bufputs of a string litteral */
#define BUFPUTSL(output, literal) \
	bufput(output, literal, sizeof literal - 1)

/* bufgrow: increasing the allocated size to the given value */
DLLEXPORT extern int bufgrow(struct buf *, size_t);

/* bufnew: allocation of a new buffer; use the system default heap allocation functions */
DLLEXPORT extern struct buf *bufnew(size_t) __attribute__ ((malloc));

/* bufnewcb: allocation of a new buffer; use user-specified heap allocation functions to manage the buffer */
DLLEXPORT extern struct buf *bufnewcb(size_t, sd_malloc_cb, sd_realloc_cb, sd_free_cb);

/* bufnullterm: NUL-termination of the string array (making a C-string) */
DLLEXPORT extern const char *bufcstr(struct buf *);

/* bufprefix: compare the beginning of a buffer with a string */
DLLEXPORT extern int bufprefix(const struct buf *buf, const char *prefix);

/* bufput: appends raw data to a buffer */
DLLEXPORT extern void bufput(struct buf *, const void *, size_t);

/* bufputs: appends a NUL-terminated string to a buffer */
DLLEXPORT extern void bufputs(struct buf *, const char *);

/* bufputc: appends a single char to a buffer */
DLLEXPORT extern void bufputc(struct buf *, int);

/* bufrelease: decrease the reference count and free the buffer if needed */
DLLEXPORT extern void bufrelease(struct buf *);

/* bufreset: frees internal data of the buffer */
DLLEXPORT extern void bufreset(struct buf *);

/* bufslurp: removes a given number of bytes from the head of the array */
DLLEXPORT extern void bufslurp(struct buf *, size_t);

/* bufprintf: formatted printing to a buffer */
DLLEXPORT extern void bufprintf(struct buf *, const char *, ...) __attribute__ ((format (printf, 2, 3)));

#ifdef __cplusplus
}
#endif

#endif
