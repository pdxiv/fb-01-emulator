.PHONY: all configure build clean test install submodules standalone

BUILD_DIR := build
BUILD_TYPE ?= Release
JOBS ?= $(shell nproc)

all: build

configure:
	@mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

build: configure
	cmake --build $(BUILD_DIR) --target fb01 -j$(JOBS)

standalone:
	@mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DFB01_BUILD_STANDALONE=ON
	cmake --build $(BUILD_DIR) --target fb01-standalone -j$(JOBS)

clean:
	rm -rf $(BUILD_DIR)

test: build
	cd $(BUILD_DIR) && ctest --output-on-failure

install: build
	@mkdir -p ~/.vst3
	cp -r $(BUILD_DIR)/VST3/$(BUILD_TYPE)/fb01.vst3 ~/.vst3/

submodules:
	git submodule update --init --recursive
