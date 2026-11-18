# ------------------------------------------------------------------
#  Makefile  --  build the maze project
#  Windows :  mingw32-make        (then run: maze.exe [seed])
#  Linux/mac:  make               (then run: ./maze [seed])
# ------------------------------------------------------------------

CC     = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2
TARGET = maze
OBJ    = main.o maze.o stack.o queue.o

ifeq ($(OS),Windows_NT)
  RM  = del /Q
  EXE = .exe
  RUN = $(TARGET).exe
else
  RM  = rm -f
  EXE =
  RUN = ./$(TARGET)
endif

$(TARGET)$(EXE): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET)$(EXE) $(OBJ)

# every .o depends on maze.h, so changing a struct rebuilds everything
main.o: main.c maze.h
	$(CC) $(CFLAGS) -c main.c
maze.o: maze.c maze.h
	$(CC) $(CFLAGS) -c maze.c
stack.o: stack.c maze.h
	$(CC) $(CFLAGS) -c stack.c
queue.o: queue.c maze.h
	$(CC) $(CFLAGS) -c queue.c

.PHONY: run clean
run: $(TARGET)$(EXE)
	$(RUN)

clean:
	-$(RM) $(OBJ) $(TARGET) $(TARGET).exe
