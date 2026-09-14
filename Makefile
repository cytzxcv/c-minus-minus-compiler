.PHONY: all clean build run
.SILENT:

all: bin/cmmc

bin/cmmc: obj/main.o
	mkdir -p bin
	g++ -std=c++23 $^ -o $@

obj/main.o: src/main.cpp
	mkdir -p obj
	g++ -std=c++23 -c $< -o $@

build: bin/cmmc

run: bin/cmmc
	./bin/cmmc

clean:
	rm -rf bin/ obj/