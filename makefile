default: all

all: b9 $(b9_programs)

executables = b9 b9test
b9_objects = main.o b9jit.o b9.o b9hash.o
b9test_objects = b9.o b9jit.o b9test.o b9hash.o
b9_programs = program.so bench.so test.so

omr_dir := ./omr
jitbuilder_dir := $(omr_dir)/jitbuilder/release
jitbuilder_includes := -I$(jitbuilder_dir)/include -I$(jitbuilder_dir)/include/compiler
jitbuilder_lib := $(jitbuilder_dir)/libjitbuilder.a

ldflags := -ldl -lm $(jitbuilder_lib)
cxxflags = -std=c++11 -fPIC -fno-rtti -rdynamic $(jitbuilder_includes)

$(foreach program,$(executables),$(eval $(program): $($(program)_objects)))

%.cpp: %.src
	node b9.js $^ > $@

%.o : %.cpp $(jitbuilder_lib)
	$(CXX) -o $@ -c $< $(cxxflags) $(CXXFLAGS)

%.so: %.o
	$(CXX) -shared -o $@ $^ $(LDFLAGS)

$(executables):
	$(CXX) -o $@ $^ $(cxxflags) $(CXXFLAGS) $(ldflags) $(LDFLAGS)

$(jitbuilder_lib): $(omr_dir)
	(cd $(omr_dir)/; ./configure)
	(cd $(omr_dir)/jitbuilder; make)

program: program.so program.cpp
	cat program.cpp
	./b9 -debug program.so

bench: b9 bench.so
	./b9 bench.so

test: b9 test.so b9test $(b9_programs)
	./b9 test.so
	./b9test

clean:
	$(RM) $(executables)
	$(RM) $(b9_objects)
	$(RM) $(b9_programs)

format:
	clang-format -i $(wildcard *.cpp *.hpp)

tidy:
	clang-tidy -extra-arg="$(cxxflags)" $(wildcard *.cpp *.hpp)

.PHONY: program bench test clean format tidy

