#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${1:-build}"
GENERATOR="${2:-Ninja}"

CMAKE_ARGS="-DEXD_INTERACTION_BUILD_TESTS=ON -DCMAKE_POLICY_VERSION_MINIMUM=3.5"

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

echo "==> Configuring with tests enabled..."
cmake -S . -B "${BUILD_DIR}" -G "${GENERATOR}" ${CMAKE_ARGS}

echo "==> Building..."
cmake --build "${BUILD_DIR}"

echo "==> Running tests..."
if [ -f "${BUILD_DIR}/CTestTestfile.cmake" ]; then
    ctest --test-dir "${BUILD_DIR}" --output-on-failure
else
    echo "==> No tests defined yet."
fi
