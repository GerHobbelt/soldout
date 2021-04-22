#include "stack.h"

#include "buffer.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

void
sd_stack_init(sd_stack *st, size_t initial_size)
{
	assert(st);

	st->item = NULL;
	st->size = st->asize = 0;

	if (!initial_size)
		initial_size = 8;

	sd_stack_grow(st, initial_size);
}

void
sd_stack_uninit(sd_stack *st)
{
	assert(st);

	free(st->item);
}

void
sd_stack_grow(sd_stack *st, size_t neosz)
{
	assert(st);

	if (st->asize >= neosz)
		return;

	st->item = sd_realloc(st->item, neosz * sizeof(void *));
	memset(st->item + st->asize, 0x0, (neosz - st->asize) * sizeof(void *));

	st->asize = neosz;

	if (st->size > neosz)
		st->size = neosz;
}

void
sd_stack_push(sd_stack *st, void *item)
{
	assert(st);

	if (st->size >= st->asize)
		sd_stack_grow(st, st->size * 2);

	st->item[st->size++] = item;
}

void *
sd_stack_pop(sd_stack *st)
{
	assert(st);

	if (!st->size)
		return NULL;

	return st->item[--st->size];
}

void *
sd_stack_top(const sd_stack *st)
{
	assert(st);

	if (!st->size)
		return NULL;

	return st->item[st->size - 1];
}
