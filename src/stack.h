/* stack.h - simple stacking */

#ifndef UPSKIRT_STACK_H
#define UPSKIRT_STACK_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


/*********
 * TYPES *
 *********/

struct sd_stack {
	void **item;
	size_t size;
	size_t asize;
};
typedef struct sd_stack sd_stack;


/*************
 * FUNCTIONS *
 *************/

/* sd_stack_init: initialize a stack */
void sd_stack_init(sd_stack *st, size_t initial_size);

/* sd_stack_uninit: free internal data of the stack */
void sd_stack_uninit(sd_stack *st);

/* sd_stack_grow: increase the allocated size to the given value */
void sd_stack_grow(sd_stack *st, size_t neosz);

/* sd_stack_push: push an item to the top of the stack */
void sd_stack_push(sd_stack *st, void *item);

/* sd_stack_pop: retrieve and remove the item at the top of the stack */
void *sd_stack_pop(sd_stack *st);

/* sd_stack_top: retrieve the item at the top of the stack */
void *sd_stack_top(const sd_stack *st);


#ifdef __cplusplus
}
#endif

#endif /** UPSKIRT_STACK_H **/
