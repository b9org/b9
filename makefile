default: all

all: b9

top_srcdir := ./omr
IJIT = $(top_srcdir)/jitbuilder/release/include
IJIT1 = $(IJIT)/compiler
LIB=$(top_srcdir)/jitbuilder/release/libjitbuilder.a

OBJ=main.o b9jit.o

b9:  main.o  b9jit.o $(LIB)
	c++ -std=c++11 -g -fPIC -fno-rtti  $(OBJ) -lm -lm $(LIB) -o b9 $(LIBS) -ldl

main.o:  main.cpp  b9.h
	c++ -std=c++11 -g -fPIC -fno-rtti  main.cpp  -c -o main.o $()

b9jit.o: b9jit.cpp b9jit.hpp $(INCLUDE_DEPS)
	c++ -std=c++11 -g -fPIC -fno-rtti -c b9jit.cpp -o b9jit.o -I$(IJIT) -I$(IJIT1) 

$(LIB):
	(cd $(top_srcdir)/jitbuilder; make)

bench: b9
	time  ./b9

clean:
	$(RM) b9
