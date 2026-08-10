#!/usr/bin/env bash
set -eux

export Qt6_DIR=/usr/lib64/cmake/Qt6
export CMAKE_PREFIX_PATH=/usr${CMAKE_PREFIX_PATH:+:$CMAKE_PREFIX_PATH}

echo "Qt6_DIR=$Qt6_DIR"
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
