{
  description = "Raspberry Pi Pico W development environment with Rust";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    rust-overlay = {
      url = "github:oxalica/rust-overlay";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, rust-overlay, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        overlays = [ (import rust-overlay) ];
        pkgs = import nixpkgs {
          inherit system overlays;
        };

        # Rust toolchain with ARM Cortex-M0+ target for Pico
        rustToolchain = pkgs.rust-bin.stable.latest.default.override {
          extensions = [ "rust-src" "rust-analyzer" "clippy" ];
          targets = [ "thumbv6m-none-eabi" ];
        };

        # Custom picotool runner for NixOS
        uf2Runner = pkgs.writeShellScript "uf2-runner" ''
          set -e
          BINARY="$1"
          ELF_BINARY="$BINARY.elf"
          
          # Copy with .elf extension so picotool recognizes it
          cp "$BINARY" "$ELF_BINARY"
          
          echo "Flashing $ELF_BINARY with picotool..."
          if sudo ${pkgs.picotool}/bin/picotool info >/dev/null 2>&1; then
            sudo ${pkgs.picotool}/bin/picotool load "$ELF_BINARY"
            sudo ${pkgs.picotool}/bin/picotool reboot
            echo "✅ Flash complete!"
            rm "$ELF_BINARY"  # Clean up
          else
            echo "⚠️  Pico not detected in bootloader mode"
            echo "   Hold BOOTSEL button while plugging in Pico, then retry"
          fi
        '';

        # Environment variables for Pico development
        picoEnvVars = {
          CARGO_TARGET_THUMBV6M_NONE_EABI_RUNNER = "${uf2Runner}";
          DEFMT_LOG = "debug";
        };
      in
      {
        devShells.default = pkgs.mkShell (picoEnvVars // {
          buildInputs = with pkgs; [
            # Rust toolchain
            rustToolchain
            cargo-generate
            cargo-binutils
            
            # Embedded development tools
            picotool
            flip-link
            
            # Build dependencies
            pkg-config
            
            # Additional tools
            minicom
            screen
          ];

          shellHook = ''
            # Display toolchain info
            echo "🦀 Rust Pico W development environment loaded!"
            echo "Rust version: $(rustc --version)"
            echo "Available targets: thumbv6m-none-eabi"
            echo ""
            echo "Quick start:"
            echo "  cargo generate --git https://github.com/rp-rs/rp2040-project-template"
            echo ""
            echo "Flashing workflow:"
            echo "  1. Hold BOOTSEL button while plugging in Pico W"
            echo "  2. cargo run --release"
            echo "  3. Pico will restart automatically!"
            echo ""
          '';
        });

        # Template for a basic Pico W + WS2812B project
        templates.pico-ws2812 = {
          path = ./template;
          description = "Raspberry Pi Pico W with WS2812B LED strip template";
        };
      });
}
