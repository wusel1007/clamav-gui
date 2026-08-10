#!/usr/bin/env bash
set -eux

# Debug: wo ist Qt6Config.cmake wirklich im Flatpak Build-Container?
QT6CONFIG=$(find /usr /app -type f -path '*/cmake/Qt6/Qt6Config.cmake' 2>/dev/null | head -n 20 || true)
echo "QT6CONFIG candidates:"
echo "${QT6CONFIG}"

QT6CONFIG_FIRST=$(echo "$QT6CONFIG" | head -n 1 || true)
if [ -z "$QT6CONFIG_FIRST" ]; then
  echo "Qt6Config.cmake nicht gefunden im Container."
  find /usr /app -type f -name 'Qt6Config.cmake' 2>/dev/null | head -n 50 || true
  exit 1
fi

Qt6_DIR=$(dirname "$QT6CONFIG_FIRST")

echo "Using Qt6_DIR=$Qt6_DIR"
ls -la "$Qt6_DIR/Qt6Config.cmake"

# Um CMake nichts "anderes" nehmen zu lassen:
unset Qt6_DIR_OVERRIDE || true
export Qt6_DIR
export CMAKE_PREFIX_PATH="$(dirname "$Qt6_DIR")"

mkdir -p build
cd build

echo "CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH"
echo "Qt6_DIR=$Qt6_DIR"

cmake -S .. -B . \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/app \
  -DQT_MAJOR_VERSION=6 \
  -DQt6_DIR="$Qt6_DIR" \
  -DCMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH" \
  --debug-find

# Jetzt bauen/installieren
cmake --build . -j"$(nproc)"
cmake --install . --prefix=/app
