/*
 * =====================================================================
 *  main.c  --  the conductor: it owns nothing, it only calls things
 * ---------------------------------------------------------------------
 *  The demo runs in two phases, because they teach different lessons:
 *
 *  PHASE A  a perfect maze (a tree: one path between any two cells)
 *           -> BFS and DFS must agree on the path; they differ only in
 *              how many cells they had to look at.
 *
 *  PHASE B  the same maze with a few extra passages punched in (loops)
 *           -> now several paths exist, and the choice of structure
 *              changes the ANSWER: BFS stays shortest, DFS wanders.
 * =====================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "maze.h"

/* path buffers live here, sized to the largest possible path */
static cell bfs_path[ROWS * COLS];
static cell dfs_path[ROWS * COLS];

/* print a path as coordinates: (0,0) -> (1,0) -> ... */
static void print_route(const char *label, const cell path[], int n)
{
    printf("%s: %d steps\n", label, n - 1);
    for (int i = 0; i < n; i++)
    {
        printf("(%d,%d)", path[i].r, path[i].c);
        if (i + 1 < n)
            printf(" -> ");
    }
    printf("\n");
}

/* Self-check: is what the solver returned really a walkable path?
 * Every step must be a neighbouring cell joined by an OPEN wall.
 * A program that prints a pretty picture can still be wrong -- this
 * proves the parent-pointer reconstruction is honest. */
static int path_is_valid(const cell path[], int n, cell start, cell end)
{
    if (n < 2)
        return 0;
    if (path[0].r != start.r || path[0].c != start.c)
        return 0;
    if (path[n - 1].r != end.r || path[n - 1].c != end.c)
        return 0;

    for (int i = 0; i + 1 < n; i++)
    {
        int dr = path[i + 1].r - path[i].r;
        int dc = path[i + 1].c - path[i].c;

        int dir = -1;
        if (dr == -1 && dc == 0) dir = N;
        if (dr ==  0 && dc == 1) dir = E;
        if (dr ==  1 && dc == 0) dir = S;
        if (dr ==  0 && dc == -1) dir = W;

        if (dir < 0)
            return 0;                        /* not a neighbour       */
        if (maze[path[i].r][path[i].c].wall[dir] != OPEN)
            return 0;                        /* a wall is in the way  */
    }
    return 1;                                /* every step is legal   */
}

/* ---------------------------------------------------------------- */
/*  BENCHMARK MODE: maze.exe -test [mazes] [loops]                   */
/*  Runs the full generate + solve cycle on many fresh mazes and    */
/*  tabulates BFS vs DFS. With loops = 0 every row must be a tie    */
/*  (tree: one path). With loops, BFS must never lose -- if the     */
/*  "DFS shorter" column ever shows anything but 0, the BFS solver  */
/*  has a bug, because BFS is mathematically guaranteed shortest.   */
/* ---------------------------------------------------------------- */
static void run_benchmark(int n, int loops)
{
    cell start = { 0, 0 };
    cell end   = { ROWS - 1, COLS - 1 };

    int bfs_wins = 0, ties = 0, dfs_shorter = 0, invalid = 0;

    printf("=== benchmark: %d random mazes, %d loops each ===\n\n", n, loops);
    printf("  maze | BFS steps | DFS steps | result\n");

    for (int i = 1; i <= n; i++)
    {
        srand((unsigned int)i);      /* maze #i is always the same maze */
        maze_init();
        maze_generate();
        maze_braid(loops);

        int b = maze_solve_bfs(start, end, bfs_path, ROWS * COLS);
        int d = maze_solve_dfs(start, end, dfs_path, ROWS * COLS);

        int b_ok = path_is_valid(bfs_path, b, start, end);
        int d_ok = path_is_valid(dfs_path, d, start, end);

        const char *result;
        if (!b_ok || !d_ok) { result = "INVALID!";        invalid++;     }
        else if (d > b)     { result = "BFS shorter";     bfs_wins++;    }
        else if (d == b)    { result = "tie";             ties++;        }
        else                { result = "DFS shorter?!";    dfs_shorter++; }

        printf("%6d | %9d | %9d | %s\n",
               i, (b > 0 ? b - 1 : b), (d > 0 ? d - 1 : d), result);
    }

    printf("\n---\n");
    printf("mazes tested : %d\n", n);
    printf("BFS shorter  : %d\n", bfs_wins);
    printf("tie          : %d\n", ties);
    printf("DFS shorter  : %d   (must stay 0: BFS is guaranteed shortest)\n", dfs_shorter);
    printf("invalid paths: %d   (must stay 0: the check walks every path)\n", invalid);
}

/* print how to use the two modes */
static void print_usage(void)
{
    printf("usage:\n");
    printf("  maze.exe                    full demo on a random maze\n");
    printf("  maze.exe <seed>             full demo on maze <seed> (repeatable)\n");
    printf("  maze.exe <seed> <loops>     full demo with <loops> extra holes\n");
    printf("  maze.exe -test <mazes> <loops>   BFS vs DFS comparison table\n");
    printf("  maze.exe -help              this screen\n");
}

int main(int argc, char *argv[])
{
    /* help screen */
    if (argc > 1 && (strcmp(argv[1], "-help") == 0 || strcmp(argv[1], "-h") == 0))
    {
        print_usage();
        return 0;
    }

    /* benchmark first: it manages its own seeds and mazes */
    if (argc > 1 && strcmp(argv[1], "-test") == 0)
    {
        int n = argc > 2 ? atoi(argv[2]) : 50;
        if (n < 1)
            n = 50;
        int test_loops = argc > 3 ? atoi(argv[3]) : maze_cells() / 10;
        run_benchmark(n, test_loops);
        return 0;
    }

    /* ---- 0. seed ---------------------------------------------------- */
    unsigned int seed = (unsigned int)time(NULL);
    if (argc > 1)
        seed = (unsigned int)strtoul(argv[1], NULL, 10);  /* repeat a maze */
    srand(seed);

    /* optional arguments:  maze.exe [seed] [extra passages]
     *   maze.exe           -> random seed, default number of loops
     *   maze.exe 7         -> repeat the maze you got with seed 7
     *   maze.exe 7 0       -> seed 7, no loops (pure perfect maze)
     * Trying several loop counts is the experiment behind the report. */
    int loops = maze_cells() / 10;            /* ~10 loops on a 10x10 */
    if (argc > 2)
        loops = atoi(argv[2]);

    printf("=== Maze %d x %d   seed %u   (full demo) ===\n", ROWS, COLS, seed);
    printf("(want the BFS vs DFS comparison table? run: maze.exe -test 20 10)\n\n");

    cell start = { 0, 0 };
    cell end   = { ROWS - 1, COLS - 1 };
    maze_set_goals(start, end);     /* so the printer can mark S and E */

    /* ================================================================ */
    /*  PHASE A -- perfect maze (tree)                                   */
    /* ================================================================ */
    maze_init();
    maze_generate();

    printf("\n--- A1. generated maze (perfect: a tree) ---\n");
    maze_print(0);

    int n_bfs = maze_solve_bfs(start, end, bfs_path, ROWS * COLS);
    int bfs_seen = maze_reached();

    int n_dfs = maze_solve_dfs(start, end, dfs_path, ROWS * COLS);
    int dfs_seen = maze_reached();

    printf("\n--- A2. BFS with the QUEUE: shortest path ---\n");
    if (n_bfs < 0)
        printf("NO PATH found\n");
    else
        maze_print(1);

    printf("\n--- A3. DFS with the STACK: any path ---\n");
    if (n_dfs < 0)
        printf("NO PATH found\n");
    else
        maze_print(1);

    printf("\nBFS : %2d steps, looked at %3d of %3d cells\n",
           n_bfs - 1, bfs_seen, maze_cells());
    printf("DFS : %2d steps, looked at %3d of %3d cells\n",
           n_dfs - 1, dfs_seen, maze_cells());
    printf("In a tree there is only ONE path, so both must return it.\n");
    printf("The difference is the work done to find it: %s looked at fewer cells\n",
           dfs_seen < bfs_seen ? "DFS" : (bfs_seen < dfs_seen ? "BFS" : "neither (a tie)"));

    /* ================================================================ */
    /*  PHASE B -- punch loops, then compare again                      */
    /* ================================================================ */
    maze_braid(loops);

    int n_bfs2 = maze_solve_bfs(start, end, bfs_path, ROWS * COLS);
    int n_dfs2 = maze_solve_dfs(start, end, dfs_path, ROWS * COLS);

    printf("\n--- B1. same maze + %d extra passages (loops) ---\n", loops);
    maze_print(0);

    printf("\nBFS : %2d steps (still the shortest)\n", n_bfs2 - 1);
    printf("DFS : %2d steps (%s)\n", n_dfs2 - 1,
           n_dfs2 > n_bfs2 ? "longer - it wandered" :
           n_dfs2 == n_bfs2 ? "same this time" : "shorter? impossible");

    if (n_dfs2 > n_bfs2)
    {
        printf("\n--- B2. the DFS path, so you can see the wandering ---\n");
        maze_print(1);
        print_route("DFS route", dfs_path, n_dfs2);
    }

    /* ================================================================ */
    /*  PHASE C -- is the graph really what we claim?                   */
    /* ================================================================ */
    printf("\n--- C. graph / path checks ---\n");
    printf("cells                : %d\n", maze_cells());
    printf("passages now         : %d\n", maze_passages());
    printf("tree after braiding? : %s\n",
           maze_passages() == maze_cells() - 1 ? "yes" : "no (loops added, as planned)");
    printf("BFS path valid       : %s\n",
           path_is_valid(bfs_path, n_bfs2, start, end) ? "yes" : "NO");
    printf("DFS path valid       : %s\n",
           path_is_valid(dfs_path, n_dfs2, start, end) ? "yes" : "NO");

    return 0;
}