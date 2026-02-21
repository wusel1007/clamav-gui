{
  description = "C++ with Qt development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
    ...
  }:
    flake-utils.lib.eachDefaultSystem (
      system: let
        pkgs = nixpkgs.legacyPackages.${system};
        qt = pkgs.qt6; # Use Qt6 (can change to qt5 if needed)
      in {
        devShells.default = pkgs.mkShell {
          # Build inputs (required during build)
          nativeBuildInputs = with pkgs; [
            cmake
            ninja
            pkg-config
          ];

          # Runtime dependencies (available in shell)
          buildInputs = with pkgs; [
            qt.full
            qt.qtbase
            qt.qttools
            qt.qtdeclarative
            gcc
            clang-tools
            gdb
          ];

          # Environment variables for Qt
          QT_PLUGIN_PATH = "${qt.qtbase}/${qt.qtbase.qtPluginPrefix}";

          # Shell hook
          shellHook = ''
            echo "C++/Qt development shell"
            echo "CMake version: $(cmake --version | head -n 1)"
            echo "Qt version: $(qmake --version | head -n 1)"
          '';
        };
      }
    );
}
