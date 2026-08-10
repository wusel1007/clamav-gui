#!/usr/bin/env bash
set -eux

mkdir -p build
cd build

cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/app \
  -DCMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH"

cmake --build . -j"$(nproc)"
cmake --install . --prefix=/app
