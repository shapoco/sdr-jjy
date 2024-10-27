.PHONY: all install clean

REPO_DIR=$(shell pwd)
SRC_DIR=.
BUILD_DIR=build

BIN_NAME=pico2_quad_detection.uf2
BIN=$(BUILD_DIR)/$(BIN_NAME)

CORE_DIR=../../core
FATFS_DIR=../fatfs/source

SRC_LIST=\
	$(wildcard src/*.*) \
	$(wildcard src/jjy/*.*)

all: $(BIN)

$(BIN): $(SRC_LIST) CMakeLists.txt
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) \
		&& cmake -DPICO_BOARD=pico2 .. \
		&& make -j
	@echo "------------------------------"
	@echo "UF2 File:"
	@echo $(REPO_DIR)/$(BIN)
	@ls -l $(REPO_DIR)/$(BIN)

install: $(BIN)
	sudo mkdir -p /mnt/e
	sudo mount -t drvfs e: /mnt/e
	cp $(BIN) /mnt/e/.

clean:
	rm -rf build
