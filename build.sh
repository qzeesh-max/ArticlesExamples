#!/usr/bin/env bash
set -e

# Base directory
BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${BASE_DIR}/build"

echo "============================================="
echo "  Unified C++ Examples Bundle: Linux Build  "
echo "============================================="

# Create build directory
if [ ! -d "${BUILD_DIR}" ]; then
    echo "Creating build directory: ${BUILD_DIR}"
    mkdir -p "${BUILD_DIR}"
fi

# Configure
echo "Configuring projects with CMake..."
cmake -B "${BUILD_DIR}" -S "${BASE_DIR}"

# Build
echo "Building all projects..."
cmake --build "${BUILD_DIR}"

echo "============================================="
echo "  Build Completed Successfully!             "
echo "============================================="
