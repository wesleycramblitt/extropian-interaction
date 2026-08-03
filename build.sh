#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${1:-build}"
GENERATOR="${2:-Ninja}"

CMAKE_ARGS="-DCMAKE_POLICY_VERSION_MINIMUM=3.5"

# If local sibling repos exist, use them directly
if [ -d "../extropian-core" ]; then
    CMAKE_ARGS="${CMAKE_ARGS} -DEXD_CORE_DIR=$(realpath ../extropian-core)"
    echo "==> Using local extropian-core at ../extropian-core"
fi
if [ -d "../extropian-geometry" ]; then
    CMAKE_ARGS="${CMAKE_ARGS} -DEXD_GEOMETRY_DIR=$(realpath ../extropian-geometry)"
    echo "==> Using local extropian-geometry at ../extropian-geometry"
fi
if [ -d "../extropian-render" ]; then
    CMAKE_ARGS="${CMAKE_ARGS} -DEXD_RENDER_DIR=$(realpath ../extropian-render)"
    echo "==> Using local extropian-render at ../extropian-render"
fi

echo "==> Configuring (${GENERATOR})..."
cmake -S . -B "${BUILD_DIR}" -G "${GENERATOR}" ${CMAKE_ARGS}

echo "==> Building..."
cmake --build "${BUILD_DIR}"

echo "==> Build complete: ${BUILD_DIR}/libexd-interaction.a"
