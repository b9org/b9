default: all

all: b9

top_srcdir := ./omr
IJIT = $(top_srcdir)/jitbuilder/release/include
IJIT1 = $(IJIT)/compiler
LIB=$(top_srcdir)/jitbuilder/release/libjitbuilder.a

OBJ=main.o b9jit.o

$(OBJ): omr

b9:  $(OBJ) $(LIB)
	$(CXX) -std=c++11 -fPIC -fno-rtti  $(OBJ) -lm -lm $(LIB) -o b9 $(LIBS) -ldl -I./omr/compiler/ $(CFLAGS)

main.o:  main.cpp  b9.h
	$(CXX) -std=c++11 -fPIC -fno-rtti  main.cpp  -c -o main.o $() -I./omr/compiler/ $(CFLAGS)

b9jit.o: b9jit.cpp b9jit.hpp b9.h $(INCLUDE_DEPS)
	$(CXX) -std=c++11 -fPIC -fno-rtti -c b9jit.cpp -o b9jit.o -I$(IJIT) -I$(IJIT1) -I./omr/compiler/ $(CFLAGS)

sharedlib:  b9
	clang -std=c++11 -shared -undefined dynamic_lookup -o test_sub.so test_sub.cpp
	./b9 test_sub

program:  b9 b9.js program.src
	node b9.js program.src >program.cpp
	cat program.cpp
	clang -std=c++11  -shared -undefined dynamic_lookup -o program.so program.cpp
	./b9 program

$(LIB):
	(cd $(top_srcdir)/jitbuilder; make)

bench: b9
	time  ./b9

clean:
	$(RM) b9
	$(RM) $(OBJ)

omr:
	if [ -d ../omr ]; then ln -s ../omr ./omr; else git clone git@github.com:eclipse/omr.git; fi
