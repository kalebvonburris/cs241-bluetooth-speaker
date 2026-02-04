# Bluetooth Speaker

## Compiling

### With Nix (Linux/MacOS)

It is recommended to install `Nix` if you are on Linux or MacOS as this project provides a `flake.nix`.

To compile and flash, run the following command in the root directory of this repo:

```bash
nix develop
```

This will start a development environment that you can build and flash the Raspberry Pi Pico W with. Connect the device while holding the `BOOTSEL` button and then run:

```bash
$ nix develop
...

🦀 Rust Pico W development environment loaded!
Rust version: rustc 1.93.0 (254b59607 2026-01-19)
Available targets: thumbv6m-none-eabi

Quick start:
  cargo generate --git https://github.com/rp-rs/rp2040-project-template

Flashing workflow:
  1. Hold BOOTSEL button while plugging in Pico W
  2. cargo run --release
  3. Pico will restart automatically!

$ cargo run --release -- -P {PORT}
```
