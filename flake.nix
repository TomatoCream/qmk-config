{
  description = "QMK keyboard definition for the YK DO52 Pro (RP2040 + TrackPoint)";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        devShells.default = pkgs.mkShell {
          packages = [
            pkgs.qmk
            pkgs.gcc-arm-embedded
            pkgs.gnumake
            pkgs.patch
            pkgs.python3
            pkgs.git
            pkgs.udisks
            pkgs.util-linux
            pkgs.usbutils
          ];

          shellHook = ''
            echo "DO52 Pro QMK config"
            echo "  qmk: $(qmk --version 2>/dev/null || echo missing)"
            echo "  arm: $(arm-none-eabi-gcc --version 2>/dev/null | head -1 || echo missing)"
            echo
            echo "  ./build.sh [keymap]   build (installs keyboard + PS/2 patch)"
            echo "  ./flash.sh            wait for RPI-RP2 and copy the UF2"
            echo "  QMK_HOME=/path/to/qmk_firmware to override the checkout"
          '';
        };

        formatter = pkgs.nixpkgs-fmt;
      }
    );
}
