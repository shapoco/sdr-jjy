.PHONY: all images fonts install launch-openocd clean distclean

REPO_DIR = ../..
LIB_DIR = $(REPO_DIR)/lib
SRC_DIR = src
BUILD_DIR = build

BIN_NAME = jjymon.uf2
BIN = $(BUILD_DIR)/$(BIN_NAME)

APP_NAMESPACE = shapoco::jjymon

BMP_DIR = bmp
IMAGES_CPP = $(SRC_DIR)/images.cpp
IMAGES_HPP = $(SRC_DIR)/images.hpp
IMAGES_SRC_LIST = $(wildcard $(BMP_DIR)/*.*)
IMAGES_CPP_GEN_CMD = ./gen_bmp_array.py

FONT4_NAME = font4
FONT5_NAME = font5
FONT12_NAME = font12
FONT16_NAME = font16

FONT_LIB_DIR = $(LIB_DIR)/shapoco/graphics
FONT_SRC_DIR = $(SRC_DIR)/fonts
FONT_COMMON_HPP = $(FONT_LIB_DIR)/tinyfont.hpp
FONT_BMP_DIR = $(BMP_DIR)/fonts
FONT_CPP_GEN_CMD = $(FONT_LIB_DIR)/gen_font_array.py

FONT_HPP_LIST = \
	$(FONT_SRC_DIR)/$(FONT4_NAME).hpp \
	$(FONT_SRC_DIR)/$(FONT5_NAME).hpp \
	$(FONT_SRC_DIR)/$(FONT12_NAME).hpp \
	$(FONT_SRC_DIR)/$(FONT16_NAME).hpp

FONT_CPP_LIST = \
	$(FONT_SRC_DIR)/$(FONT4_NAME).cpp \
	$(FONT_SRC_DIR)/$(FONT5_NAME).cpp \
	$(FONT_SRC_DIR)/$(FONT12_NAME).cpp \
	$(FONT_SRC_DIR)/$(FONT16_NAME).cpp

SRC_LIST=\
	$(wildcard $(SRC_DIR)/*.*) \
	$(wildcard $(SRC_DIR)/fonts/*.*) \
	$(wildcard $(LIB_DIR)/shapoco/*.*) \
	$(wildcard $(LIB_DIR)/shapoco/graphics/*.*) \
	$(wildcard $(LIB_DIR)/shapoco/pico/*.*) \
	$(wildcard $(LIB_DIR)/shapoco/jjy/*.*) \
	$(wildcard $(LIB_DIR)/shapoco/jjy/rx/*.*)

all: $(BIN)
fonts: $(FONT_HPP_LIST)
images: $(IMAGES_HPP)

$(BIN): $(SRC_LIST) $(FONT_HPP_LIST) CMakeLists.txt
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) \
		&& cmake -DPICO_BOARD=pico2 -DCMAKE_BUILD_TYPE=Debug .. \
		&& make -j
	@echo "------------------------------"
	@echo "UF2 File:"
	@echo $(BIN)
	@ls -l $(BIN)

$(IMAGES_HPP): $(IMAGES_CPP) $(IMAGES_SRC_LIST) $(IMAGES_CPP_GEN_CMD)
$(IMAGES_CPP): $(IMAGES_SRC_LIST) $(IMAGES_CPP_GEN_CMD)
	rm -f $(IMAGES_CPP) $(IMAGES_HPP)
	@echo "#ifndef JJYMON_IMAGES_HPP" >> $(IMAGES_HPP)
	@echo "#define JJYMON_IMAGES_HPP" >> $(IMAGES_HPP)
	@echo >> $(IMAGES_HPP)
	@echo "#include <stdint.h>" >> $(IMAGES_HPP)
	@echo "namespace $(APP_NAMESPACE) {" >> $(IMAGES_HPP)
	@echo >> $(IMAGES_HPP)
	@echo "#include <stdint.h>" >> $(IMAGES_CPP)
	@echo "namespace $(APP_NAMESPACE) {" >> $(IMAGES_CPP)
	$(IMAGES_CPP_GEN_CMD) --outcpp $(IMAGES_CPP) --outhpp $(IMAGES_HPP) --src $(BMP_DIR)/bit_icons.png --name bmp_bit_icons
	$(IMAGES_CPP_GEN_CMD) --outcpp $(IMAGES_CPP) --outhpp $(IMAGES_HPP) --src $(BMP_DIR)/meter_frame.png --name bmp_meter_frame
	$(IMAGES_CPP_GEN_CMD) --outcpp $(IMAGES_CPP) --outhpp $(IMAGES_HPP) --src $(BMP_DIR)/icon_beat.png --name bmp_icon_beat
	@echo >> $(IMAGES_HPP)
	@echo "}" >> $(IMAGES_HPP)
	@echo "#endif" >> $(IMAGES_HPP)
	@echo >> $(IMAGES_CPP)
	@echo "}" >> $(IMAGES_CPP)

$(FONT_SRC_DIR)/$(FONT4_NAME).hpp : $(FONT_SRC_DIR)/$(FONT4_NAME).cpp $(FONT_BMP_DIR)/$(FONT4_NAME).png
$(FONT_SRC_DIR)/$(FONT4_NAME).cpp : $(FONT_BMP_DIR)/$(FONT4_NAME).png $(FONT_CPP_GEN_CMD)
	$(FONT_CPP_GEN_CMD) \
		--src $< \
		--name $(FONT4_NAME) \
		--outdir $(FONT_SRC_DIR) \
		--cpp_namespace $(APP_NAMESPACE)::fonts \
		--code-offset 32

$(FONT_SRC_DIR)/$(FONT5_NAME).hpp : $(FONT_SRC_DIR)/$(FONT5_NAME).cpp $(FONT_BMP_DIR)/$(FONT5_NAME).png
$(FONT_SRC_DIR)/$(FONT5_NAME).cpp : $(FONT_BMP_DIR)/$(FONT5_NAME).png $(FONT_CPP_GEN_CMD)
	$(FONT_CPP_GEN_CMD) \
		--src $< \
		--name $(FONT5_NAME) \
		--outdir $(FONT_SRC_DIR) \
		--cpp_namespace $(APP_NAMESPACE)::fonts \
		--code-offset 32

$(FONT_SRC_DIR)/$(FONT12_NAME).hpp : $(FONT_SRC_DIR)/$(FONT12_NAME).cpp $(FONT_BMP_DIR)/$(FONT12_NAME).png
$(FONT_SRC_DIR)/$(FONT12_NAME).cpp : $(FONT_BMP_DIR)/$(FONT12_NAME).png $(FONT_CPP_GEN_CMD)
	$(FONT_CPP_GEN_CMD) \
		--src $< \
		--name $(FONT12_NAME) \
		--outdir $(FONT_SRC_DIR) \
		--cpp_namespace $(APP_NAMESPACE)::fonts \
		--code-offset 32 \
		--spacing 1

$(FONT_SRC_DIR)/$(FONT16_NAME).hpp : $(FONT_SRC_DIR)/$(FONT16_NAME).cpp $(FONT_BMP_DIR)/$(FONT16_NAME).png
$(FONT_SRC_DIR)/$(FONT16_NAME).cpp : $(FONT_BMP_DIR)/$(FONT16_NAME).png $(FONT_CPP_GEN_CMD)
	$(FONT_CPP_GEN_CMD) \
		--src $< \
		--name $(FONT16_NAME) \
		--outdir $(FONT_SRC_DIR) \
		--cpp_namespace $(APP_NAMESPACE)::fonts \
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
	rm -f $(IMAGES_HPP) $(IMAGES_CPP)

distclean:
	rm -rf build
