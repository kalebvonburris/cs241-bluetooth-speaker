{
  description = "Pico W Bluetooth speaker (C/C++ / pico-sdk)";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
    let
      system = builtins.currentSystem;
      pkgs   = nixpkgs.legacyPackages.${system};

      pico-sdk = pkgs.fetchFromGitHub {
        owner           = "raspberrypi";
        repo            = "pico-sdk";
        rev             = "2.2.0";
        hash            = "sha256-8ubZW6yQnUTYxQqYI6hi7s3kFVQhe5EaxVvHmo93vgk=";
        fetchSubmodules = true;
      };
    in {
      devShells.${system}.default = pkgs.mkShell {
        packages = with pkgs; [
          gcc-arm-embedded
          cmake
          ninja
          python3
          picotool
        ];
        shellHook = ''
          export PICO_SDK_PATH="${pico-sdk}"
          export PICO_BOARD="pico_w"
          echo "PICO_SDK_PATH=$PICO_SDK_PATH"
        '';
      };
    };
}
