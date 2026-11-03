#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
NDK_ROOT="${ANDROID_NDK_HOME:-${ANDROID_NDK_ROOT:-}}"

if [ -z "${NDK_ROOT}" ]; then
  printf 'Set ANDROID_NDK_HOME or ANDROID_NDK_ROOT before building.\n' >&2
  exit 1
fi
TOOLCHAIN_FILE="${NDK_ROOT}/build/cmake/android.toolchain.cmake"
if [ ! -f "${TOOLCHAIN_FILE}" ]; then
  printf 'Android toolchain file not found: %s\n' "${TOOLCHAIN_FILE}" >&2
  exit 1
fi

cmake -S "${SCRIPT_DIR}" -B "${BUILD_DIR}" \
  -DANDROID_PLATFORM=31 \
  -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_FILE}" \
  -DANDROID_ABI=arm64-v8a \
  -DCMAKE_BUILD_TYPE=Release
cmake --build "${BUILD_DIR}"

printf 'Release build completed: %s/fuzz\n' "${BUILD_DIR}"
