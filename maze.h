#ifndef MAZE_H      /* this file is created only if it is not there yet */
#define MAZE_H      /* and from now on, this file is "defined"             */

/*
 * =====================================================================
 *  maze.h  --  everything the whole program shares
 * ---------------------------------------------------------------------
 *  A header holds DECLARATIONS only: constants, types, prototypes.
 *  The real code (the DEFINITIONS) lives in the .c files.
 *  Rule: a name is declared in maze.h and defined exactly once in a .c
 * =====================================================================
 */

/* ---------- 1. size of the grid ------------------------------------- */
/* #ifndef lets you change the size at compile time without editing:
 *     gcc -DROWS=15 -DCOLS=20 ...
 * (Keeping the number in ONE place is why these are here, not in maze.c)
 */
#ifndef ROWS
#define ROWS 10
#endif
#ifndef COLS
#define COLS 10
#endif

/* ---------- 2. directions ------------------------------------------- */
/* the four sides of a cell, used as the index into wall[4] */
enum { N, E, S, W };                 /* N=0, E=1, S=2, W=3 */

/* what a wall slot can hold */
enum { OPEN = 0, WALL = 1 };

/* ---------- 3. a position in the grid ------------------------------- */
typedef struct
{
    int r, c;                        /* row, column */
} cell;

/* ---------- 4. one linked-list box, shared by stack and queue ------- */
/* The box is identical for both structures; only the rule of "which
 * end do I touch" is different. That is why Node is shared.
 */
typedef struct Node
{
    cell data;               /* the value this box carries   */
    struct Node *next;               /* pointer to the next box      */
} Node;

typedef struct { Node *top; } Stack;   /* LIFO: top only   */
typedef struct { Node *front, *rear;} Queue;   /* FIFO: two ends   */

/* ---------- 5. one square of the maze ------------------------------- */
typedef struct
{
    int  visited;     /* 0 = not seen yet, 1 = seen                    */
    int  wall[4];     /* 1 = wall, 0 = open passage (index: N,E,S,W)   */
    int  on_path;     /* 1 = part of the final solution                */
    cell parent;      /* which cell we came from (the tree edge)       */
} MazeCell;

/*
 * extern means: "this variable exists, but it is created somewhere else."
 * So this line does NOT reserve RAM. The array is allocated once, in
 * maze.c:        MazeCell maze[ROWS][COLS];
 * Every other file only gets a reference to that one array.
 * Without extern you would get a separate copy per .c file.
 */
extern MazeCell maze[ROWS][COLS];

/* ---------- 6. stack.c  (linked-list stack) ------------------------- */
void stack_init (Stack *s);
void stack_push (Stack *s, cell x);
int  stack_pop  (Stack *s, cell *out);      /* 1 = popped, 0 = empty     */
int  stack_empty(const Stack *s);
void stack_free (Stack *s);

/* ---------- 7. queue.c  (linked-list queue) ------------------------- */
void queue_init (Queue *q);
void queue_push (Queue *q, cell x);
int  queue_pop  (Queue *q, cell *out);      /* 1 = popped, 0 = empty     */
int  queue_empty(const Queue *q);
void queue_free (Queue *q);

/* ---------- 8. maze.c  (grid, generate, solve, print) --------------- */
void maze_init     (void);                  /* all walls up, nothing seen*/
void maze_generate (void);                  /* DFS carve -> spanning tree*/
int  maze_solve_bfs(cell start, cell end, cell path[], int max_path);
int  maze_solve_dfs(cell start, cell end, cell path[], int max_path);
void maze_set_goals(cell start, cell end);  /* cells the printer marks S/E */
void maze_braid    (int extra);             /* punch extra holes = loops */
void maze_print    (int show_path);         /* 1 = draw the solution     */
int  maze_cells    (void);                  /* ROWS * COLS               */
int  maze_passages (void);                  /* number of open passages   */
int  maze_reached  (void);                  /* cells last search saw     */

#endif
