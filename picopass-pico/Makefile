SHELL := /bin/sh

BUILD_DIR ?= build
PICO_BOARD ?= pico_w
PICO_SDK_PATH ?= $(HOME)/pico-sdk
PICO_SECRET ?= PICOPASS
PICO_DEBUG ?= 0

CMAKE_ARGS ?=

.PHONY: all build configure clean distclean help

all: build

help:
	@echo "Targets:"
	@echo "  make|make build      Configure (if needed) and build"
	@echo "  make clean           Remove build outputs (keeps build dir)"
	@echo "  make distclean       Remove build directory"
	@echo ""
	@echo "Variables:"
	@echo "  PICO_BOARD=$(PICO_BOARD)"
	@echo "  PICO_SDK_PATH=$(PICO_SDK_PATH)"
	@echo "  PICO_SECRET=$(PICO_SECRET)"
	@echo "  PICO_DEBUG=$(PICO_DEBUG)"
	@echo "  BUILD_DIR=$(BUILD_DIR)"
	@echo "  CMAKE_ARGS=$(CMAKE_ARGS)"


$(BUILD_DIR)/CMakeCache.txt:
	@mkdir -p "$(BUILD_DIR)"
	@if [ ! -d "$(PICO_SDK_PATH)" ]; then \
		echo "Error: Pico SDK not found at: $(PICO_SDK_PATH)"; \
		echo "Set it via: export PICO_SDK_PATH=/path/to/pico-sdk"; \
		echo "or run: make CMAKE_ARGS='-DPICO_SDK_FETCH_FROM_GIT=ON'"; \
		exit 1; \
	fi
	@cmake -S . -B "$(BUILD_DIR)" \
		-DPICO_BOARD="$(PICO_BOARD)" \
		-DPICO_SDK_PATH="$(PICO_SDK_PATH)" \
		-DPICO_SECRET="$(PICO_SECRET)" \
		-DPICOPASS_DEBUG=$(PICO_DEBUG)
		$(CMAKE_ARGS)

configure: $(BUILD_DIR)/CMakeCache.txt

build: configure
	@cmake --build "$(BUILD_DIR)" -j

clean:
	@cmake --build "$(BUILD_DIR)" --target clean >/dev/null 2>&1 || true
	@rm -f "$(BUILD_DIR)"/*.uf2 "$(BUILD_DIR)"/*.elf "$(BUILD_DIR)"/*.bin "$(BUILD_DIR)"/*.hex

distclean:
	@rm -rf "$(BUILD_DIR)"
