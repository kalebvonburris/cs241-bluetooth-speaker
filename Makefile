.PHONY: all flash clean

all:
	cmake -B build -G Ninja -DPICO_BOARD=pico_w
	cmake --build build

flash:
	cmake -B build -G Ninja -DPICO_BOARD=pico_w
	cmake --build build --target flash

clean:
	rm -rf build
