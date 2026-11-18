/*
 * =====================================================================
 *  queue.c  --  a queue made of a linked list
 * ---------------------------------------------------------------------
 *  Rule of a queue: FIRST in, FIRST out. Add at the rear, remove at the
 *  front -- like a line at a ticket counter.
 *  Why TWO pointers? front alone can remove in O(1) but to ADD you would
 *  have to walk to the last box (O(n)). With rear already pointing at the
 *  last box, adding is O(1) too. Both ends are cheap, nothing shifts.
 * =====================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include "maze.h"

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

/* empty queue: nothing at the front, nothing at the rear */
void queue_init(Queue *q)
{
    q->front = NULL;
    q->rear  = NULL;
}

int queue_empty(const Queue *q)
{
    return q->front == NULL;
}

/* PUSH (enqueue): add at the rear.
 * Two cases matter: empty queue (new box is both ends) and non-empty
 * (old rear points at it, then rear moves to it). */
void queue_push(Queue *q, cell x)
{
    Node *n = (Node *)checked_malloc(sizeof(Node));
    n->data = x;
    n->next = NULL;             /* the rear box never points anywhere */

    if (q->rear == NULL)        /* queue was empty */
    {
        q->front = n;
        q->rear  = n;
    }
    else
    {
        q->rear->next = n;      /* old last box now points at new one */
        q->rear       = n;      /* rear is the new box                */
    }
}

/* POP (dequeue): remove from the front.
 * If that was the LAST box, rear must also become NULL, otherwise rear
 * would point at freed memory -- the classic linked-queue bug. */
int queue_pop(Queue *q, cell *out)
{
    if (q->front == NULL)
        return 0;

    Node *n = q->front;
    *out    = n->data;
    q->front = n->next;
    if (q->front == NULL)       /* we just removed the only box */
        q->rear = NULL;
    free(n);
    return 1;
}

void queue_free(Queue *q)
{
    cell ignored;
    while (queue_pop(q, &ignored))
        ;
}
