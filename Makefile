BUILD_FOLDER ?= ./build
BUILD_TYPE ?= Debug

build:
	$(MAKE) clean
	cmake -S . -B $(BUILD_FOLDER) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	cmake --build $(BUILD_FOLDER) -j$(shell nproc)

release:
	$(MAKE) build BUILD_TYPE=Release

debug:
	$(MAKE) build BUILD_TYPE=Debug

clean:
	rm -rf $(BUILD_FOLDER)
