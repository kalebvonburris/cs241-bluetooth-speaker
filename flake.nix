{
  description = "Pico W Bluetooth speaker (arduino-pico / arduino-cli)";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };
  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let pkgs = nixpkgs.legacyPackages.${system}; in { 
        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            arduino-cli
            python3
            picotool
          ];

          nativeBuildInputs = with pkgs; [
            udev
          ];

          buildInputs = with pkgs; [
            pkg-config
          ];

          shellHook = ''
            BOARD_URL="https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json"

            # Install arduino-pico core if not already present
            if ! arduino-cli core list 2>/dev/null | grep -q "rp2040:rp2040"; then
              echo "Installing arduino-pico core..."
              arduino-cli core update-index --additional-urls "$BOARD_URL"
              arduino-cli core install rp2040:rp2040 --additional-urls "$BOARD_URL"
            fi

            export LD_LIBRARY_PATH="${pkgs.udev}/lib:$LD_LIBRARY_PATH"

            echo ""
            echo "║   Pico W BT Speaker dev shell ready  ║"
            echo "╚══════════════════════════════════════╝"
            echo "  make        — compile"
            echo "  make flash  — compile + flash"
            echo "  make clean  — wipe build/"
            echo ""
          '';
        };
      }
    );
}
