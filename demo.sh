#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${1:-build}"
GENERATOR="${2:-Ninja}"

CMAKE_ARGS="-DBUILD_DEMO=ON -DCMAKE_POLICY_VERSION_MINIMUM=3.5"

if [ -d "../extropian-core" ]; then
    CMAKE_ARGS="${CMAKE_ARGS} -DEXD_CORE_DIR=$(realpath ../extropian-core)"
fi
if [ -d "../extropian-geometry" ]; then
    CMAKE_ARGS="${CMAKE_ARGS} -DEXD_GEOMETRY_DIR=$(realpath ../extropian-geometry)"
fi
if [ -d "../extropian-render" ]; then
    CMAKE_ARGS="${CMAKE_ARGS} -DEXD_RENDER_DIR=$(realpath ../extropian-render)"
fi

DEMO_BIN="$(realpath ${BUILD_DIR})/demo/extropian-interaction-demo"

if [ ! -f "${DEMO_BIN}" ]; then
    echo "==> Configuring with demo enabled..."
    cmake -S . -B "${BUILD_DIR}" -G "${GENERATOR}" ${CMAKE_ARGS}
    echo "==> Building..."
    cmake --build "${BUILD_DIR}" --target extropian-interaction-demo
    echo ""
fi

echo "==> Demo binary: ${DEMO_BIN}"
echo "    (Run directly — no shader dependency for interaction demo)"
exec "${DEMO_BIN}"
