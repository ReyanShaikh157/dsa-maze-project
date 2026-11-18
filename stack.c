/*
 * =====================================================================
 *  stack.c  --  a stack made of a linked list
 * ---------------------------------------------------------------------
 *  Rule of a stack: LAST in, FIRST out. The newest item is on top and
 *  it is the only one you can reach. Push and pop both touch the head,
 *  so both cost O(1) and nothing ever has to shift along.
 *  The list can grow and shrink to any size -> no fixed capacity.
 * =====================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include "maze.h"

/* one place to handle "out of memory" instead of testing everywhere */
static void *checked_malloc(size_t bytes)
{
    void *p = malloc(bytes);
    if (p == NULL)
    {
        fprintf(stderr, "out of memory\n");
        exit(EXIT_FAILURE);
    }
    return p;
}

/* start with an empty stack: no top at all */
void stack_init(Stack *s)
{
    s->top = NULL;
}

/* is there anything to pop? */
int stack_empty(const Stack *s)
{
    return s->top == NULL;      /* == is true(1) when the stack is empty */
}

/* PUSH: make a new box, put it in front of the old top, move top up. */
void stack_push(Stack *s, cell x)
{
    Node *n = (Node *)checked_malloc(sizeof(Node));
    n->data = x;
    n->next = s->top;           /* new box points at the old top  */
    s->top  = n;                /* top is now the new box         */
}

/* POP: hand back the top value, unlink the box, free its memory.
 * Returns 0 (and leaves *out alone) when the stack is empty, so the
 * caller cannot accidentally read garbage. */
int stack_pop(Stack *s, cell *out)
{
    if (s->top == NULL)
        return 0;

    Node *n = s->top;           /* remember the box we are removing */
    *out     = n->data;         /* copy the value out               */
    s->top   = n->next;         /* top drops to the second box      */
    free(n);                    /* give the memory back             */
    return 1;
}

/* empty the stack completely (used before the program exits) */
void stack_free(Stack *s)
{
    cell ignored;
    while (stack_pop(s, &ignored))
        ;                       /* pop until nothing is left */
}
