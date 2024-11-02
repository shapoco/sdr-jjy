.PHONY: all bmpfont install launch-openocd clean distclean

REPO_DIR=$(shell pwd)
SRC_DIR=.
BUILD_DIR=build

BIN_NAME=pico2_jjy_receiver.uf2
BIN=$(BUILD_DIR)/$(BIN_NAME)

CORE_DIR=../../core
FATFS_DIR=../fatfs/source

FONT16_NAME = font16

FONT_INC_DIR = bmpfont
FONT_SRC_DIR = src/$(FONT_INC_DIR)
FONT_COMMON_HPP = $(FONT_SRC_DIR)/common.hpp
FONT_BMP_DIR = bmp
FONT_CPP_GEN_CMD = ./gen_font_array.py

FONT_HPP_LIST = \
	$(FONT_SRC_DIR)/$(FONT16_NAME).hpp

FONT_CPP_LIST = \
	$(FONT_SRC_DIR)/$(FONT16_NAME).cpp

SRC_LIST=\
	$(wildcard src/*.*) \
	$(wildcard src/jjy/*.*) \
	$(wildcard src/jjy/rx/*.*) \
	$(FONT_COMMON_HPP)

all: $(BIN)
bmpfont: $(FONT_HPP_LIST)

$(BIN): $(SRC_LIST) $(FONT_HPP_LIST) CMakeLists.txt
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) \
		&& cmake -DPICO_BOARD=pico2 -DCMAKE_BUILD_TYPE=Debug .. \
		&& make -j
	@echo "------------------------------"
	@echo "UF2 File:"
	@echo $(REPO_DIR)/$(BIN)
	@ls -l $(REPO_DIR)/$(BIN)

$(FONT_SRC_DIR)/$(FONT16_NAME).hpp : $(FONT_SRC_DIR)/$(FONT16_NAME).cpp
$(FONT_SRC_DIR)/$(FONT16_NAME).cpp : $(FONT_BMP_DIR)/$(FONT16_NAME).png $(FONT_CPP_GEN_CMD)
	$(FONT_CPP_GEN_CMD) \
		--src-image $< \
		--name $(FONT16_NAME) \
		--outdir $(FONT_SRC_DIR) \
		--incdir $(FONT_INC_DIR) \
		--height 16 \
		--code-offset 32

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
	rm -f $(BIN)
	rm -f $(FONT_HPP_LIST) $(FONT_CPP_LIST)

distclean:
	rm -rf build
