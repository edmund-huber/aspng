.DEFAULT_GOAL = bin/aspng

CXX = g++
CXXFLAGS = -std=c++17 -pedantic -Wall -Wextra -Wpedantic -Wcast-align -Werror
CORE_OBJECTS = obj/aspng.o obj/aspng_surface.o obj/background_device.o obj/base_template_device.o obj/bounding_box.o obj/bridge_device.o obj/coord.o obj/copper_device.o obj/device.o obj/electrical_value.o obj/input_device.o obj/led_device.o obj/net.o obj/patch.o obj/png.o obj/port.o obj/pull_device.o obj/simple_aspng_surface.o obj/sink_device.o obj/source_device.o obj/switch_device.o obj/transistor_device.o
CORE_OBJECTS_WASM = $(CORE_OBJECTS:.o=.ow)

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

# See https://github.com/emscripten-ports/SDL2/issues/130#issuecomment-851701543
# for why we're using DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=0.
bin/aspng_wasm.js: CXX = emcc $(CXXFLAGS) -s WASM=1 -s MODULARIZE=1 -s USE_LIBPNG -s USE_SDL=2 -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=0
bin/aspng_wasm.js: obj/gui.ow $(CORE_OBJECTS_WASM) | bin
	$(CXX) $^ --embed-file tests/full_ripple_adder/_.png@input.png --embed-file font.png -o $@

obj/%.ow: %.cpp | obj
	$(CXX) -c $(CXXFLAGS) $^ -o $@

.PHONY: clean
clean:
	rm -r bin/ obj/
