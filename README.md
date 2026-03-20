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

## License

This project is dual-licensed under the Apache License, Version 2.0 and the MIT License.

You may choose to use this software under the terms of either:

- Apache License, Version 2.0 (see `LICENSE-APACHE-2.0`)
- MIT License (see `LICENSE-MIT`)

at your option.
