#!/usr/bin/env bash
set -euo pipefail

BUILD_TYPE="${1:-Release}"
BUILD_DIR="${2:-build}"

cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" "${JUCE_PATH:+-DJUCE_PATH=$JUCE_PATH}"
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"

echo ""
echo "Build complete. Look for the plugin in:"
echo "  $BUILD_DIR/fl_humbucker_artefacts/$BUILD_TYPE/VST3/"
