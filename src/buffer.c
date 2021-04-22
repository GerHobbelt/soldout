#include "buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void *
sd_malloc(size_t size)
{
	void *ret = malloc(size);

	if (!ret) {
		fprintf(stderr, "Allocation failed.\n");
		abort();
	}

	return ret;
}

void *
sd_calloc(size_t nmemb, size_t size)
{
	void *ret = calloc(nmemb, size);

	if (!ret) {
		fprintf(stderr, "Allocation failed.\n");
		abort();
	}

	return ret;
}

void *
sd_realloc(void *ptr, size_t size)
{
	void *ret = realloc(ptr, size);

	if (!ret) {
		fprintf(stderr, "Allocation failed.\n");
		abort();
	}

	return ret;
}

void
sd_buffer_init(
	sd_buffer *buf,
	size_t unit,
	sd_realloc_callback data_realloc,
	sd_free_callback data_free,
	sd_free_callback buffer_free)
{
	assert(buf);

	buf->data = NULL;
	buf->size = buf->asize = 0;
	buf->unit = unit;
	buf->data_realloc = data_realloc;
	buf->data_free = data_free;
	buf->buffer_free = buffer_free;
}

void
sd_buffer_uninit(sd_buffer *buf)
{
	assert(buf && buf->unit);
	buf->data_free(buf->data);
}

sd_buffer *
sd_buffer_new(size_t unit)
{
	sd_buffer *ret = sd_malloc(sizeof (sd_buffer));
	sd_buffer_init(ret, unit, sd_realloc, free, free);
	return ret;
}

void
sd_buffer_free(sd_buffer *buf)
{
	if (!buf) return;
	assert(buf && buf->unit);

	buf->data_free(buf->data);

	if (buf->buffer_free)
		buf->buffer_free(buf);
}

void
sd_buffer_reset(sd_buffer *buf)
{
	assert(buf && buf->unit);

	buf->data_free(buf->data);
	buf->data = NULL;
	buf->size = buf->asize = 0;
}

void
sd_buffer_grow(sd_buffer *buf, size_t neosz)
{
	size_t neoasz;
	assert(buf && buf->unit);

	if (buf->asize >= neosz)
		return;

	neoasz = buf->asize + buf->unit;
	while (neoasz < neosz)
		neoasz += buf->unit;

	buf->data = buf->data_realloc(buf->data, neoasz);
	memset(&buf->data[buf->asize], 0, neoasz - buf->asize);
	buf->asize = neoasz;
}

void
sd_buffer_put(sd_buffer *buf, const uint8_t *data, size_t size)
{
	if (!size)
		return;
	assert(buf && buf->unit);

	if (buf->size + size > buf->asize)
		sd_buffer_grow(buf, buf->size + size);

	memcpy(buf->data + buf->size, data, size);
	buf->size += size;
}

void
sd_buffer_puts(sd_buffer *buf, const char *str)
{
	if (!str)
		return;
	sd_buffer_put(buf, (const uint8_t *)str, strlen(str));
}

void
sd_buffer_putc(sd_buffer *buf, uint8_t c)
{
	assert(buf && buf->unit);

	if (buf->size >= buf->asize)
		sd_buffer_grow(buf, buf->size + 1);

	buf->data[buf->size] = c;
	buf->size += 1;
}

int
sd_buffer_putf(sd_buffer *buf, FILE *file)
{
	assert(buf && buf->unit);

	while (!(feof(file) || ferror(file))) {
		sd_buffer_grow(buf, buf->size + buf->unit);
		buf->size += fread(buf->data + buf->size, 1, buf->unit, file);
	}

	return ferror(file);
}

void
sd_buffer_set(sd_buffer *buf, const uint8_t *data, size_t size)
{
	assert(buf && buf->unit);

	if (size > buf->asize)
		sd_buffer_grow(buf, size);

	memcpy(buf->data, data, size);
	buf->size = size;
}

void
sd_buffer_sets(sd_buffer *buf, const char *str)
{
	sd_buffer_set(buf, (const uint8_t *)str, strlen(str));
}

int
sd_buffer_eq(const sd_buffer *buf, const uint8_t *data, size_t size)
{
	if (buf->size != size) return 0;
	return memcmp(buf->data, data, size) == 0;
}

int
sd_buffer_eqs(const sd_buffer *buf, const char *str)
{
	return sd_buffer_eq(buf, (const uint8_t *)str, strlen(str));
}

int
sd_buffer_prefix(const sd_buffer *buf, const char *prefix)
{
	size_t i;

	for (i = 0; i < buf->size; ++i) {
		if (prefix[i] == 0)
			return 0;

		if (buf->data[i] != prefix[i])
			return buf->data[i] - prefix[i];
	}

	return 0;
}

void
sd_buffer_slurp(sd_buffer *buf, size_t size)
{
	assert(buf && buf->unit);

	if (size >= buf->size) {
		buf->size = 0;
		return;
	}

	buf->size -= size;
	memmove(buf->data, buf->data + size, buf->size);
}

const char *
sd_buffer_cstr(sd_buffer *buf)
{
	assert(buf && buf->unit);

	if (buf->size < buf->asize && buf->data[buf->size] == 0)
		return (char *)buf->data;

	sd_buffer_grow(buf, buf->size + 1);
	buf->data[buf->size] = 0;

	return (char *)buf->data;
}

void
sd_buffer_printf(sd_buffer *buf, const char *fmt, ...)
{
	va_list ap;
	int n;

	assert(buf && buf->unit);

	if (buf->size >= buf->asize)
		sd_buffer_grow(buf, buf->size + 1);

	va_start(ap, fmt);
	n = vsnprintf((char *)buf->data + buf->size, buf->asize - buf->size, fmt, ap);
	va_end(ap);

	if (n < 0) {
#ifndef _MSC_VER
		return;
#else
		va_start(ap, fmt);
		n = _vscprintf(fmt, ap);
		va_end(ap);
#endif
	}

	if ((size_t)n >= buf->asize - buf->size) {
		sd_buffer_grow(buf, buf->size + n + 1);

		va_start(ap, fmt);
		n = vsnprintf((char *)buf->data + buf->size, buf->asize - buf->size, fmt, ap);
		va_end(ap);
	}

	if (n < 0)
		return;

	buf->size += n;
}

void sd_buffer_put_utf8(sd_buffer *buf, unsigned int c) {
	unsigned char unichar[4];

	assert(buf && buf->unit);

	if (c < 0x80) {
		sd_buffer_putc(buf, c);
	}
	else if (c < 0x800) {
		unichar[0] = 192 + (c / 64);
		unichar[1] = 128 + (c % 64);
		sd_buffer_put(buf, unichar, 2);
	}
	else if (c - 0xd800u < 0x800) {
		UPSKIRT_BUFPUTSL(buf, "\xef\xbf\xbd");
	}
	else if (c < 0x10000) {
		unichar[0] = 224 + (c / 4096);
		unichar[1] = 128 + (c / 64) % 64;
		unichar[2] = 128 + (c % 64);
		sd_buffer_put(buf, unichar, 3);
	}
	else if (c < 0x110000) {
		unichar[0] = 240 + (c / 262144);
		unichar[1] = 128 + (c / 4096) % 64;
		unichar[2] = 128 + (c / 64) % 64;
		unichar[3] = 128 + (c % 64);
		sd_buffer_put(buf, unichar, 4);
	}
	else {
		UPSKIRT_BUFPUTSL(buf, "\xef\xbf\xbd");
	}
}

void sd_buffer_replace_last(sd_buffer *buf, const char * str)
{
	int n = strlen(str);
	if (n > buf->size)
	{
		return;
	}
	int i;
	for (i = 1;i <= n;i++){
		buf->data[buf->size-i] = str[n-i];
	}
}
