.PHONY: all build run b+r clean

BUILD_DIR := build
EXECUTABLE := $(BUILD_DIR)/bin/zhiva
URL ?= "https://wxn0.xyz"

all: build

build:
	@echo "Building the project..."
	@mkdir -p $(BUILD_DIR)
	@cmake -B $(BUILD_DIR) -S .
	@$(MAKE) -C $(BUILD_DIR)

run:
	@echo "Running the application..."
	@$(EXECUTABLE) $(URL)

br: build run

clean:
	@echo "Cleaning the build directory..."
	@rm -rf $(BUILD_DIR)
