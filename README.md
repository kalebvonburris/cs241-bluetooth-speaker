# Bluetooth Speaker

## Compiling

### With Nix (Linux/MacOS)

It is recommended to install `Nix` if you are on Linux or MacOS as this project provides a `flake.nix`.

Enter the development environment from the project root:

```bash
nix develop
```

Then build the project:

```bash
make        # build
make flash  # build + flash
make clean  # wipe build dir
```

The Pico W will reboot automatically and begin running the firmware.
