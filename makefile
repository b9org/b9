default: all

all: b9

top_srcdir := ./omr
IJIT = $(top_srcdir)/jitbuilder/release/include
IJIT1 = $(IJIT)/compiler
LIB=$(top_srcdir)/jitbuilder/release/libjitbuilder.a

OBJ=main.o b9jit.o

b9:  $(OBJ) $(LIB)
	c++ -fPIC -fno-rtti  $(OBJ) -lm -lm $(LIB) -o b9 $(LIBS) -ldl -I./omr/compiler/

main.o:  main.cpp  b9.h
	c++ -fPIC -fno-rtti  main.cpp  -c -o main.o $() -I./omr/compiler/

b9jit.o: b9jit.cpp b9jit.hpp b9.h $(INCLUDE_DEPS)
	c++ -fPIC -fno-rtti -c b9jit.cpp -o b9jit.o -I$(IJIT) -I$(IJIT1) -I./omr/compiler/

sharedlib:  b9
	clang -std=c++11 -shared -undefined dynamic_lookup -o test_sub.so test_sub.cpp
	./b9 test_sub

gen:  b9 b9.js test_sub.src
	node b9.js test_sub.src

$(LIB):
	(cd $(top_srcdir)/jitbuilder; make)

bench: b9
	time  ./b9

clean:
	$(RM) b9
	$(RM) $(OBJ)
