{
  pkgs? import <nixpkgs> {}
}:
let 
  src = pkgs.lib.cleanSource ./.;
in 
pkgs.stdenv.mkDerivation {
  pname = "clamav-gui";
  version = "25.05.0";

  LOCALE_ARCHIVE = "${pkgs.glibcLocales}/lib/locale/locale-archive";
  LANG = "en_US.UTF-8";
  LC_ALL = "en_US.UTF-8";

  inherit src;

  # Debug: Verify files are copied correctly
  preConfigure = ''
    echo "=== Source tree contents ==="
    find $src
    echo "==========================="
  '';

  nativeBuildInputs = with pkgs; [
    pkg-config
    cmake
    qt6.wrapQtAppsHook
    glibcLocales  # Provides UTF-8 support
  ];
  buildInputs = with pkgs; [
    qt6.qtbase
    qt6.qtsvg
    qt6.qttools
    qt6.qtwebengine
    qt6.qt5compat
    qt6.qtwayland
  ];

  doCheck = true;

  # to prevent double wrapping of wrapQtApps and wrapGApps
  #dontWrapGApps = true;

  preFixup = ''
    qtWrapperArgs+=("''${gappsWrapperArgs[@]}")
  '';

  configurePhase = ''
    mkdir -p build
    cd build
    cmake .. \
      -DCMAKE_SOURCE_DIR=$src \
      -DCMAKE_BINARY_DIR=$(pwd) \
      -DCMAKE_INSTALL_PREFIX=$out
  '';

  # Ensure Qt environment variables are set correctly
  postInstall = ''
    wrapQtAppsHook
  '';

  meta = with pkgs.lib; {
    description = "A C++ project using CMake and Qt 6 with qt5compat";
    license = licenses.mit;
    platforms = platforms.linux;
    maintainers = [ ];
  };
}
