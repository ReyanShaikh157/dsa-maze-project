# Maze Generator and Solver

A C program that builds a random maze and solves it in two different ways,
to show that the data structure you choose changes the result.

The maze is made of squares. Some sides are walls (blocked), some are open
passages (walkable). The program first builds the maze by knocking down
walls, then finds the path from the start square S to the end square E
twice, using two different tools, and compares the two paths.

---

## How it works (3 steps)

### Step 1 - Build the maze (with a stack)

Every square starts fully walled in. The computer walks randomly: it knocks
down walls into unvisited squares, and when it gets stuck it uses a STACK
(a pile of plates) to remember how to walk back. It never opens a wall into
an already-visited square, so the finished maze has no loops - there is
exactly ONE path between any two squares. Such a maze is called a TREE.

### Step 2 - Solve it twice (queue vs stack)

- BFS with a QUEUE (a ticket line) spreads out in rings, like ripples in
  water, so the first time it reaches E is the SHORTEST possible path.
- DFS with a STACK (the pile of plates) goes as deep as it can, backing up
  when stuck. It finds A path, but not always the shortest.

### Step 3 - Compare (and prove)

In a perfect maze there is only one path, so both solvers find the same
path (they tie). Then we BRAID the maze - punch extra holes in walls to
create loops, so many paths now exist. Now the difference appears: BFS
still finds the shortest path, while DFS often wanders and finds a longer
one.

---

## The data structures and their jobs

| Structure    | Simple idea                              | Job in our project                              |
|--------------|------------------------------------------|-------------------------------------------------|
| Graph        | dots joined by lines                     | the maze itself: squares = dots, passages = lines |
| Stack        | pile of plates (last in, first out)      | building the maze, and solving it with DFS      |
| Queue        | ticket line (first in, first out)        | solving the maze with BFS (shortest path)       |
| Linked list  | toy train, each car points to the next   | the material used to build the stack and queue  |
| Tree         | connected graph with no loops            | the perfect maze; parent notes also form the path |

---

## What we found

- Perfect maze (a tree): BFS and DFS always return the SAME path, because
  only one path exists. Example: both took 60 steps.
- Braided maze (with loops): BFS always finds the SHORTEST path; DFS may
  take a longer one. Example: BFS 26 steps, DFS 62 steps, same maze.
- Tested on many mazes: in 20 random mazes, BFS never lost - it was
  shorter 18 times and tied 2 times. DFS was never shorter, which is
  mathematically guaranteed.

## Conclusion

The same maze, the same start, the same end - but the data structure
changes the answer. A QUEUE (BFS) guarantees the shortest path; a STACK
(DFS) finds any path and may wander. Choosing the right structure is what
the project demonstrates.

---

## How to run

Open a terminal in this folder, then:

    mingw32-make            build the program (or: make on Linux/macOS)
    maze.exe 3              full demo on maze number 3 (repeatable)
    maze.exe 3 0            same maze, no loops (perfect maze)
    maze.exe 3 40           same maze, 40 extra loops
    maze.exe -test 20 10    BFS vs DFS comparison on 20 random mazes
    maze.exe -help          list of all commands

Build a bigger maze if you want:

    mingw32-make clean
    mingw32-make CFLAGS="-std=c99 -Wall -Wextra -O2 -DROWS=25 -DCOLS=40"

---

## Files

| File      | Job                                                        |
|-----------|------------------------------------------------------------|
| maze.h    | the rule book: types, constants and prototypes             |
| stack.c   | linked-list stack (push / pop)                             |
| queue.c   | linked-list queue (push at rear, pop at front)             |
| maze.c    | build the maze, solve it, draw it                          |
| main.c    | the demo and the -test comparison mode                     |
| Makefile  | the recipe that builds everything                          |
