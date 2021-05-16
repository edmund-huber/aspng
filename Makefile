.DEFAULT_GOAL = bin/aspng

CXX = g++
CXXFLAGS = -std=c++17 -pedantic -Wall -Wextra -Wpedantic -Wcast-align -Werror
CORE_OBJECTS = obj/aspng.o obj/aspng_surface.o obj/background_device.o obj/base_template_device.o obj/bounding_box.o obj/bridge_device.o obj/coord.o obj/copper_device.o obj/device.o obj/electrical_value.o obj/input_device.o obj/led_device.o obj/net.o obj/patch.o obj/png.o obj/port.o obj/pull_device.o obj/simple_aspng_surface.o obj/sink_device.o obj/source_device.o obj/switch_device.o obj/transistor_device.o
CORE_OBJECTS_WASM = obj/aspng.ow obj/aspng_surface.ow obj/background_device.ow obj/base_template_device.ow obj/bounding_box.ow obj/bridge_device.ow obj/coord.ow obj/copper_device.ow obj/device.ow obj/electrical_value.ow obj/input_device.ow obj/led_device.ow obj/net.ow obj/patch.ow obj/png.ow obj/port.ow obj/pull_device.ow obj/simple_aspng_surface.ow obj/sink_device.ow obj/source_device.ow obj/switch_device.ow obj/transistor_device.ow

test: bin/test
	./bin/test

bin:
	mkdir bin

bin/test: obj/test.o $(CORE_OBJECTS) | bin
	$(CXX) $^ -lpng -lstdc++fs -o $@

bin/aspng: obj/gui.o $(CORE_OBJECTS) | bin
	$(CXX) $^ -lpng -lstdc++fs -lSDL2 -o $@

obj:
	mkdir obj

obj/%.o: %.cpp | obj
	$(CXX) -c $(CXXFLAGS) $^ -o $@

bin/aspng_wasm.js: CXX = emcc $(CXXFLAGS) -s WASM=1 -s USE_LIBPNG -s USE_SDL=2
bin/aspng_wasm.js: obj/gui.ow $(CORE_OBJECTS_WASM) | bin
	$(CXX) $^ -o $@

obj/%.ow: %.cpp | obj
	$(CXX) -c $(CXXFLAGS) $^ -o $@

.PHONY: clean
clean:
	rm -r bin/ obj/
