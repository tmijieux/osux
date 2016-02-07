#include <stdlib.h>
#include <limits.h>
#include "stack.h"

struct stack {
    int head;
    unsigned buffer_size;
    STACK_TYPE buf[];
};

size_t stack_size(const struct stack *s)
{
    return (size_t)(s->head + 1);
}

struct stack *stack_create(size_t buffer_size)
{
    struct stack *stack = malloc(sizeof(*stack) +
				 sizeof(stack->buf[0]) * buffer_size);
    stack->head = UINT_MAX;
    stack->buffer_size = buffer_size;
    return stack;
}

void stack_push(struct stack *stack, STACK_TYPE element)
{
    if ( !stack_is_full(stack) ) {
	stack->head ++;
	stack->buf[stack->head] = element;
    }
}

STACK_TYPE stack_peek(const struct stack *stack)
{
    return stack->buf[stack->head];
}

STACK_TYPE stack_pop(struct stack *stack)
{
    STACK_TYPE tmp = STACK_TYPE_INITIALIZER;
    if ( !stack_is_empty(stack) ) {
	tmp = stack->buf[stack->head];
	stack->head --;
    }
    return tmp;
}

int stack_is_empty(const struct stack *stack)
{
    return (stack->head == -1);
}

int stack_is_full(const struct stack *stack)
{
    return ((unsigned) stack->head == stack->buffer_size - 1);
}

void stack_destroy(struct stack *stack)
{
    free(stack);
}
