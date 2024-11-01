.PHONY: all install launch-openocd clean

REPO_DIR=$(shell pwd)
SRC_DIR=.
BUILD_DIR=build

BIN_NAME=pico2_jjy_receiver.uf2
BIN=$(BUILD_DIR)/$(BIN_NAME)

CORE_DIR=../../core
FATFS_DIR=../fatfs/source

SRC_LIST=\
	$(wildcard src/*.*) \
	$(wildcard src/jjy/*.*) \
	$(wildcard src/jjy/rx/*.*)

all: $(BIN)

$(BIN): $(SRC_LIST) CMakeLists.txt
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) \
		&& cmake -DPICO_BOARD=pico2 -DCMAKE_BUILD_TYPE=Debug .. \
		&& make -j
	@echo "------------------------------"
	@echo "UF2 File:"
	@echo $(REPO_DIR)/$(BIN)
	@ls -l $(REPO_DIR)/$(BIN)

install: $(BIN)
	sudo mkdir -p /mnt/e
	sudo mount -t drvfs e: /mnt/e
	cp $(BIN) /mnt/e/.

debug: $(BIN)
	gdb-multiarch -ex "target remote localhost:3333" $(BIN)

launch-openocd: $(BIN)
	cd $(OPENOCD_REPO_PATH) && \
	sudo src/openocd \
		-f tcl/interface/cmsis-dap.cfg \
		-c 'adapter speed 1000' \
		-f tcl/target/rp2350.cfg \
		-s tcl

clean:
	rm -rf build
