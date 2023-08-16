#CXX=g++

CFLAGS= #-gdb #-DDMALLOC_FUNC_CHECK
#LIBS=-lm
LIBS=-lc
LIBS+=-lncurses

sudoku: sudoku.o
	$(CXX) $(CFLAGS) $^ $(LIBS) -o $@

clean:
	rm -rf sukodu *.o *~
