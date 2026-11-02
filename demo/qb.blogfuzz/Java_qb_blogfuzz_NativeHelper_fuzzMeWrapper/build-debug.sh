#!/bin/bash
set -e

BUILD_DIR="build-debug"

mkdir -p "${BUILD_DIR}"
(
  cd "${BUILD_DIR}"
  cmake .. \
    -DANDROID_PLATFORM=31 \
    -DCMAKE_TOOLCHAIN_FILE='/Users/jcgc04/Library/Android/sdk/ndk/25.2.9519653/build/cmake/android.toolchain.cmake' \
    -DANDROID_ABI=arm64-v8a \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo
)
cmake --build "${BUILD_DIR}"

echo "Debug build completed: ${BUILD_DIR}/fuzz"
