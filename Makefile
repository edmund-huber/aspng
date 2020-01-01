.DEFAULT_GOAL=bin/aspng

CXX=g++
CXXFLAGS=-std=c++17 -pedantic -Wall -Werror -g -O0
CORE_OBJECTS=obj/background_device.o obj/copper_device.o obj/device.o obj/patch.o obj/png.o obj/pull_device.o obj/sink_device.o obj/source_device.o obj/transistor_device.o

test: bin/test
	./bin/test

bin:
	mkdir bin

bin/test: obj/test.o $(CORE_OBJECTS) | bin
	$(CXX) $^ -lpng -o $@ -lstdc++fs

bin/aspng: obj/gui.o $(CORE_OBJECTS) | bin
	$(CXX) $^ -lpng -o $@ -lstdc++fs -lSDL2

obj:
	mkdir obj

obj/%.o: %.cpp | obj
	$(CXX) -c $(CXXFLAGS) $^ -o $@

.PHONY: clean
clean:
	rm -r bin/ obj/
