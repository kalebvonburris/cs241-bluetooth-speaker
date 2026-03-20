# Detect connected board via picotool
CHIP := $(shell picotool info 2>/dev/null | grep -i "rp2350" > /dev/null && echo "rp2350" || echo "rp2040")

ifeq ($(CHIP),rp2350)
    BOARD = rp2040:rp2040:rpipico2w:ipbtstack=ipv4btcblebig
else
    BOARD = rp2040:rp2040:rpipicow:ipbtstack=ipv4btcblebig
endif

BOARD_URL = https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json

# Auto-detect the Pico W's serial port via arduino-cli
PORT := arduino-cli board list 2>/dev/null | grep "rp2040:rp2040" | awk '{print $1}' | head -1

.PHONY: all flash clean

all:
	arduino-cli compile --fqbn $(BOARD) --additional-urls $(BOARD_URL) .

flash: all
	@if [ -z "$(PORT)" ]; then \
		echo "Error: No Pico W detected. Is it plugged in?"; \
		exit 1; \
	fi
	arduino-cli upload --fqbn $(BOARD) --additional-urls $(BOARD_URL) --port $(BOARD) .

clean:
	rm -rf build/
