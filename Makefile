.PHONY: build run clean
.SILENT:

SRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:src/%.cpp=obj/%.o)

CXX=g++
CXXFLAG=-std=c++23 -Iinc -O2 -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wconversion -Wold-style-cast

build: bin/cmmc

run: bin/cmmc
	./bin/cmmc

bin/cmmc: $(OBJS)
	mkdir -p bin
	$(CXX) $(CXXFLAG) $^ -o $@

obj/%.o: src/%.cpp
	mkdir -p obj
	$(CXX) $(CXXFLAG) -c $< -o $@

clean:
	rm -rf bin/ obj/