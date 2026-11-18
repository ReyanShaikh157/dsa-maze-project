/*
 * =====================================================================
 *  maze.c  --  the grid: build it, solve it, draw it
 * ---------------------------------------------------------------------
 *  The maze IS a graph:
 *      node  = cell
 *      edge  = open passage between two neighbouring cells
 *      wall  = "there is no edge here"
 *  Generating = adding edges until every cell is reachable AND there is
 *              exactly one path between any two cells -> N cells, N-1 edges -> a tree.
 *  Solving    = searching that tree (queue = BFS = shortest path,
 *               stack = DFS = any path).
 * =====================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include "maze.h"

/* ---- THE one and only definition of the maze (see extern in maze.h) -- */
MazeCell maze[ROWS][COLS];

/* which two cells the printer marks as S (start) and E (end) */
static cell g_start = { 0, 0 };
static cell g_end   = { ROWS - 1, COLS - 1 };

/* ---- movement tables: what direction N/E/S/W does to  (row,col) ----- */
/*      N=(-1,0)   E=(0,+1)   S=(+1,0)   W=(0,-1)                       */
static const int DR[4] = { -1, 0, 1,  0 };
static const int DC[4] = {  0, 1, 0, -1 };

/* the direction seen from the other cell: N<->S, E<->W */
static int opposite(int d)
{
    return (d + 2) % 4;         /* 0<->2, 1<->3 */
}

/* a cell is only legal if it is inside the grid */
static int in_bounds(int r, int c)
{
    return r >= 0 && r < ROWS && c >= 0 && c < COLS;
}

/* ---------------------------------------------------------------- */
/*  STEP 1: start with every cell walled off and nothing visited.    */
/*  Outer walls are set to WALL and never touched again, so the      */
/*  maze can never leak off the edge of the grid.                    */
/* ---------------------------------------------------------------- */
void maze_init(void)
{
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
        {
            maze[r][c].visited = 0;
            maze[r][c].on_path = 0;
            maze[r][c].wall[N] = WALL;
            maze[r][c].wall[E] = WALL;
            maze[r][c].wall[S] = WALL;
            maze[r][c].wall[W] = WALL;
            maze[r][c].parent.r = r;    /* "I came from myself" for now */
            maze[r][c].parent.c = c;
        }
}

/* ---------------------------------------------------------------- */
/*  remove the wall between (r,c) and its neighbour in direction d.  */
/*  A wall is shared by two cells, so BOTH sides must be opened --   */
/*  forget one side and the maze becomes inconsistent (you can walk */
/*  out of a cell but not back in).                                  */
/* ---------------------------------------------------------------- */
static void carve(int r, int c, int d)
{
    int nr = r + DR[d];
    int nc = c + DC[d];

    if (!in_bounds(nr, nc))
        return;                 /* never carve through the outer border */

    maze[r][c].wall[d]            = OPEN;
    maze[nr][nc].wall[opposite(d)] = OPEN;
}

/* random shuffle of the four directions (Fisher-Yates).
 * Without this the maze would always be carved N,E,S,W -> boring. */
static void shuffle_dirs(int order[4])
{
    order[0] = N; order[1] = E; order[2] = S; order[3] = W;
    for (int i = 3; i > 0; i--)
    {
        int j = rand() % (i + 1);
        int t = order[i]; order[i] = order[j]; order[j] = t;
    }
}

/* ---------------------------------------------------------------- */
/*  STEP 2: carve the maze with the stack (DFS, "recursive           */
/*  backtracker").                                                   */
/*                                                                   */
/*  Why a stack? When you hit a dead end you must remember how you   */
/*  got there. The last cell you pushed is exactly the one to return */
/*  to, so pop gives you the backtrack for free.                     */
/*                                                                   */
/*  Trick: pop the current cell, look for an unvisited neighbour.    */
/*    found one -> push the current cell BACK, then push the         */
/*                 neighbour: the current cell waits on the stack    */
/*                 until we come back to it.                         */
/*    found none -> do not push it back: it is a dead end, we have   */
/*                 already popped our way out of it.                 */
/*  When the stack is empty, every cell has been visited.            */
/* ---------------------------------------------------------------- */
void maze_generate(void)
{
    Stack st;
    stack_init(&st);

    cell start = { 0, 0 };
    maze[start.r][start.c].visited = 1;
    stack_push(&st, start);

    while (!stack_empty(&st))
    {
        cell cur;
        stack_pop(&st, &cur);

        int order[4];
        shuffle_dirs(order);

        for (int k = 0; k < 4; k++)
        {
            int d  = order[k];
            int nr = cur.r + DR[d];
            int nc = cur.c + DC[d];

            if (!in_bounds(nr, nc) || maze[nr][nc].visited)
                continue;               /* skip the wall / the seen cell */

            carve(cur.r, cur.c, d);     /* remove the wall = add the edge */
            maze[nr][nc].visited = 1;   /* mark BEFORE pushing            */
            stack_push(&st, cur);       /* remember where we came from    */
            cell nxt = { nr, nc };
            stack_push(&st, nxt);       /* move in                        */
            break;                      /* one move per pop               */
        }
        /* no neighbour -> nothing pushed -> that cell is finished */
    }

    stack_free(&st);
}

/* ---------------------------------------------------------------- */
/*  reset everything a SEARCH touches, but keep the walls.           */
/*  Each solver starts from a clean sheet, otherwise visited[] left  */
/*  over from the generator would make BFS think it is finished.     */
/* ---------------------------------------------------------------- */
static void reset_search(void)
{
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
        {
            maze[r][c].visited = 0;
            maze[r][c].on_path = 0;
            maze[r][c].parent.r = r;
            maze[r][c].parent.c = c;
        }
}

/* ---------------------------------------------------------------- */
/*  Walk the parent pointers from end back to start, then reverse.   */
/*  This is the tree part of the answer: parent[] IS the path.       */
/*  Returns the number of cells in the path, or -1 if it does not    */
/*  fit in path[] or the chain does not reach the start.             */
/* ---------------------------------------------------------------- */
static int build_path(cell start, cell end, cell path[], int max_path)
{
    cell tmp[ROWS * COLS];
    int  n   = 0;
    cell cur = end;

    while (!(cur.r == start.r && cur.c == start.c))
    {
        if (n >= ROWS * COLS)       /* broken chain: stop, do not hang */
            return -1;
        tmp[n++] = cur;
        cur = maze[cur.r][cur.c].parent;
    }
    tmp[n++] = start;

    if (n > max_path)
        return -1;

    for (int i = 0; i < n; i++)             /* reverse into path[] */
    {
        path[i] = tmp[n - 1 - i];
        maze[path[i].r][path[i].c].on_path = 1;   /* mark for printing */
    }
    return n;
}

/* ---------------------------------------------------------------- */
/*  SOLVE with a queue (BFS).                                        */
/*                                                                   */
/*  A queue spreads like a ripple: every cell 1 step away, then 2,   */
/*  then 3. Because it never jumps ahead, the first time the end is  */
/*  reached it was reached in the fewest possible steps -> the path  */
/*  found is guaranteed SHORTEST.                                    */
/* ---------------------------------------------------------------- */
int maze_solve_bfs(cell start, cell end, cell path[], int max_path)
{
    if (!in_bounds(start.r, start.c) || !in_bounds(end.r, end.c))
        return -1;
    if (start.r == end.r && start.c == end.c)
        return -1;                  /* same cell: no path to find */

    reset_search();

    Queue q;
    queue_init(&q);

    maze[start.r][start.c].visited = 1;
    queue_push(&q, start);

    int found = 0;
    while (!queue_empty(&q))
    {
        cell cur;
        queue_pop(&q, &cur);

        if (cur.r == end.r && cur.c == end.c)
        {
            found = 1;
            break;                  /* first arrival = shortest */
        }

        for (int d = 0; d < 4; d++)
        {
            if (maze[cur.r][cur.c].wall[d] != OPEN)
                continue;           /* wall: no edge in this direction */

            int nr = cur.r + DR[d];
            int nc = cur.c + DC[d];
            if (!in_bounds(nr, nc) || maze[nr][nc].visited)
                continue;

            maze[nr][nc].visited = 1;
            maze[nr][nc].parent  = cur;     /* remember the edge */
            cell nxt = { nr, nc };
            queue_push(&q, nxt);            /* add at the REAR   */
        }
    }

    queue_free(&q);

    if (!found)
        return -1;
    return build_path(start, end, path, max_path);
}

/* ---------------------------------------------------------------- */
/*  SOLVE with a stack (DFS).                                        */
/*                                                                   */
/*  Same data structure that carved the maze, now searching it: go   */
/*  as deep as possible, backtrack at dead ends. It finds A path,    */
/*  usually a long and winding one -- compare the length with BFS to */
/*  see why the choice of structure matters.                         */
/*                                                                   */
/*  Marking visited when PUSHING (not when popping) keeps the parent */
/*  chain a real path: each cell's parent is the cell that reached   */
/*  it first, and an open passage joins the two.                     */
/* ---------------------------------------------------------------- */
int maze_solve_dfs(cell start, cell end, cell path[], int max_path)
{
    if (!in_bounds(start.r, start.c) || !in_bounds(end.r, end.c))
        return -1;
    if (start.r == end.r && start.c == end.c)
        return -1;

    reset_search();

    Stack st;
    stack_init(&st);

    maze[start.r][start.c].visited = 1;
    stack_push(&st, start);

    int found = 0;
    cell cur;

    while (stack_pop(&st, &cur))
    {
        if (cur.r == end.r && cur.c == end.c)
        {
            found = 1;
            break;
        }

        for (int d = 0; d < 4; d++)
        {
            if (maze[cur.r][cur.c].wall[d] != OPEN)
                continue;

            int nr = cur.r + DR[d];
            int nc = cur.c + DC[d];
            if (!in_bounds(nr, nc) || maze[nr][nc].visited)
                continue;

            maze[nr][nc].visited = 1;
            maze[nr][nc].parent  = cur;
            cell nxt = { nr, nc };
            stack_push(&st, nxt);       /* newest on TOP is taken next */
        }
    }

    stack_free(&st);

    if (!found)
        return -1;
    return build_path(start, end, path, max_path);
}

/* ---------------------------------------------------------------- */
/*  STEP 3 (optional): punch EXTRA passages into a finished maze.    */
/*  Each extra passage creates a cycle (a loop), so the maze is no   */
/*  longer a tree and is no longer "perfect".                         */
/*                                                                   */
/*  Why bother? In a tree there is EXACTLY ONE simple path between   */
/*  two cells, so BFS and DFS must return the same path -- the pair  */
/*  only differs in how much of the maze each looked at. The moment  */
/*  loops exist there are several paths, and THEN the structure      */
/*  changes the answer: BFS still gives the shortest, DFS may wander.*/
/* ---------------------------------------------------------------- */
void maze_braid(int extra)
{
    int guard = 0;

    while (extra > 0 && guard < ROWS * COLS * 50)
    {
        guard++;

        int r = rand() % ROWS;
        int c = rand() % COLS;
        int d = rand() % 4;

        int nr = r + DR[d];
        int nc = c + DC[d];

        if (!in_bounds(nr, nc))
            continue;                   /* never open the outer border  */
        if (maze[r][c].wall[d] == OPEN)
            continue;                   /* already open: no new cycle   */

        carve(r, c, d);
        extra--;
    }
}

/* ---------------------------------------------------------------- */
/*  HOW MANY cells the last search visited (the generator leaves     */
/*  them all visited, so call a solver first).                       */
/*  BFS and DFS touch different numbers of cells -- good evidence.   */
/* ---------------------------------------------------------------- */
int maze_reached(void)
{
    int n = 0;
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            if (maze[r][c].visited)
                n++;
    return n;
}

int maze_cells(void)
{
    return ROWS * COLS;
}

/* every open passage counted once: each cell compares its E and S wall */
int maze_passages(void)
{
    int n = 0;
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
        {
            if (maze[r][c].wall[E] == OPEN && c + 1 < COLS) n++;
            if (maze[r][c].wall[S] == OPEN && r + 1 < ROWS) n++;
        }
    return n;
}

/* tell the printer which cells are the start and the finish */
void maze_set_goals(cell start, cell end)
{
    g_start = start;
    g_end   = end;
}

/* ---------------------------------------------------------------- */
/*  DRAW the maze from the wall data.                                */
/*  Each cell is 3 characters wide:                                   */
/*      +-----+       '+' at every corner,                           */
/*      |  *  |       '---' where the N/S wall is still there,       */
/*      +-----+       '|'   where the W/E wall is still there.       */
/*  Nothing is stored about the picture -- it is generated from      */
/*  wall[4], which proves the walls are consistent.                  */
/* ---------------------------------------------------------------- */
void maze_print(int show_path)
{
    for (int r = 0; r < ROWS; r++)
    {
        /* top edge of this row */
        for (int c = 0; c < COLS; c++)
        {
            printf("+");
            printf("%s", maze[r][c].wall[N] == WALL ? "---" : "   ");
        }
        printf("+\n");

        /* the cells themselves */
        for (int c = 0; c < COLS; c++)
        {
            printf("%s", maze[r][c].wall[W] == WALL ? "|" : " ");
            if (r == g_start.r && c == g_start.c)
                printf(" S ");              /* the starting square    */
            else if (r == g_end.r && c == g_end.c)
                printf(" E ");              /* the finishing square   */
            else if (show_path && maze[r][c].on_path)
                printf(" * ");              /* part of the solution   */
            else
                printf("   ");
        }
        printf("%s\n", maze[r][COLS - 1].wall[E] == WALL ? "|" : " ");
    }

    /* bottom edge of the last row */
    for (int c = 0; c < COLS; c++)
    {
        printf("+");
        printf("%s", maze[ROWS - 1][c].wall[S] == WALL ? "---" : "   ");
    }
    printf("+\n");
}
