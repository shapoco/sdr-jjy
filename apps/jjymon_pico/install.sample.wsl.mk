
PICO_DRIVE := e

install: $(BIN)
	sudo mkdir -p /mnt/$(PICO_DRIVE)
	sudo mount -t drvfs $(PICO_DRIVE): /mnt/$(PICO_DRIVE)
	cp $(BIN) /mnt/$(PICO_DRIVE)/.
