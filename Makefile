CXX=g++
CXXFLAGS=-std=c++11 -pedantic -Wall -Werror

test: bin/test
	./bin/test

bin:
	mkdir bin

bin/test: obj/background_device.o obj/copper_device.o obj/device.o obj/png.o obj/sink_device.o obj/source_device.o obj/test.o | bin
	$(CXX) $^ -lpng -o $@

obj:
	mkdir obj

obj/%.o: %.cpp | obj
	$(CXX) -c $(CXXFLAGS) $^ -o $@

.PHONY: clean
clean:
	rm -r bin/ obj/
