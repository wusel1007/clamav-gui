#!/usr/bin/env bash
set -eux

export Qt6_DIR=/usr/lib64/cmake/Qt6
# Debug
echo "Qt6_DIR=$Qt6_DIR"
ls -la "$Qt6_DIR/Qt6Config.cmake"

mkdir -p build
cd build

cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/app \
  -DQT_MAJOR_VERSION=6

cmake --build . -j"$(nproc)"
cmake --install . --prefix=/app
