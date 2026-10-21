#!/usr/bin/env bash
# Builds Zelda on PC (a Cemu fork) on Linux with CMake and Ninja.
#
# Usage: tools/build-linux.sh [release|relwithdebinfo|debug]
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MODE="${1:-release}"
BUILD="$ROOT/build/$MODE"

if [ ! -x "$ROOT/dependencies/vcpkg/vcpkg" ]; then
  "$ROOT/dependencies/vcpkg/bootstrap-vcpkg.sh" -disableMetrics
fi

# the vcpkg manifest installs the dependencies into build/<mode>/vcpkg_installed
cmake -S "$ROOT" -B "$BUILD" -G Ninja \
  -DCMAKE_BUILD_TYPE="$MODE" \
  -DVCPKG_INSTALL_OPTIONS="--clean-after-build" \
  -DENABLE_DISCORD_RPC=OFF

cmake --build "$BUILD" -- -k 0

echo "Built: $BUILD"
