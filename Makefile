.PHONY: all clean build watch

all: build

build:
	@mkdir -p build
	@cd build && cmake .. && cmake --build .

clean:
	rm -rf build mygame/native

watch: build
	fswatch -o lib/ steam-sdk/ include/ CMakeLists.txt | xargs -n1 sh -c 'make build'
