#!/usr/bin/env bash
set -eux

# finde den Qt6Config.cmake Pfad im Container zur Build-Zeit
QT6CONFIG=$(find /usr /app -path '*/Qt6/Qt6Config.cmake' -type f 2>/dev/null | head -n 1 || true)

echo "QT6CONFIG=$QT6CONFIG"

if [ -z "$QT6CONFIG" ]; then
  echo "Qt6Config.cmake nicht gefunden. Beispielverzeichnis-Ausschnitt:"
  find /usr -path '*/cmake/Qt6/*' -maxdepth 6 -type f 2>/dev/null | head -n 20 || true
  exit 1
fi

# Qt6_DIR ist das Verzeichnis, das Qt6Config.cmake enthält
Qt6_DIR=$(dirname "$QT6CONFIG")

export Qt6_DIR
export CMAKE_PREFIX_PATH=/usr${CMAKE_PREFIX_PATH:+:$CMAKE_PREFIX_PATH}

echo "Using Qt6_DIR=$Qt6_DIR"
ls -la "$Qt6_DIR/Qt6Config.cmake"

mkdir -p build
cd build

cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/app \
  -DQT_MAJOR_VERSION=6 \
  -DQt6_DIR="$Qt6_DIR" \
  -DCMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH"

cmake --build . -j"$(nproc)"
cmake --install . --prefix=/app
