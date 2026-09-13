#!/bin/bash
# build_fuzz.sh — Build fakelua fuzz targets
#
# Architecture:
#   1. Build fakelua + fuzz_bridge with GCC (main project build, no coverage)
#   2. Compile fuzz targets with clang + libFuzzer, linking against GCC libs
#      and using GCC 15's libstdc++ to resolve C++ symbols
#
# Usage:
#   ./fuzz/build_fuzz.sh              # build fuzz targets
#   ./fuzz/build_fuzz.sh clean        # clean build dir
#   ./fuzz/build_fuzz.sh test         # quick test fuzz_compile (1k runs)
#   ./fuzz/build_fuzz.sh test-diff    # quick test fuzz_differential (1k runs)

set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build_fuzz"
FUZZ_DIR="${PROJECT_ROOT}/fuzz"
CORPUS_DIR="${FUZZ_DIR}/corpus/seed"
OUTPUT_DIR="${BUILD_DIR}/bin"

# GCC 15 paths (needed for std::to_chars, std::format, etc.)
GCC15_PREFIX="/root/GCC-15"
GCC15_LIBDIR="${GCC15_PREFIX}/lib64"

# ---- Phase 1: Build fakelua + fuzz_bridge with GCC ----
phase1_build() {
    echo ""
    echo "=== Phase 1: Building fakelua + fuzz_bridge (GCC) ==="

# Rebuild if source updated
    local bridge_lib
    if [ -f "${BUILD_DIR}/lib64/libfuzz_bridge.a" ]; then
        bridge_lib="${BUILD_DIR}/lib64/libfuzz_bridge.a"
    elif [ -f "${BUILD_DIR}/lib/libfuzz_bridge.a" ]; then
        bridge_lib="${BUILD_DIR}/lib/libfuzz_bridge.a"
    fi
    if [ -n "${bridge_lib:-}" ] && [ "${FUZZ_DIR}/fuzz_bridge.cpp" -ot "${bridge_lib}" ] \
        && [ "${FUZZ_DIR}/fuzz_bridge.h" -ot "${bridge_lib}" ]; then
        echo "fuzz_bridge up to date, skipping."
        return
    fi

    mkdir -p "${BUILD_DIR}"
    cd "${BUILD_DIR}"

    cmake "${PROJECT_ROOT}" \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_FUZZ=ON \
        -DUSE_COV=OFF \
        -DUSE_ASAN=OFF \
        2>&1 | tail -5

    cmake --build . --target fuzz_bridge -j"$(nproc)" 2>&1 | tail -5

    echo ""
    echo "[OK] Phase 1 complete"
}

# ---- Find built library paths ----
find_libs() {
    # fuzz_bridge static lib
    if [ -f "${BUILD_DIR}/lib64/libfuzz_bridge.a" ]; then
        export BRIDGE_LIBDIR="${BUILD_DIR}/lib64"
    elif [ -f "${BUILD_DIR}/lib/libfuzz_bridge.a" ]; then
        export BRIDGE_LIBDIR="${BUILD_DIR}/lib"
    else
        echo "ERROR: libfuzz_bridge.a not found"
        exit 1
    fi

    # fakelua shared lib
    if [ -f "${BUILD_DIR}/lib64/libfakelua.so" ]; then
        export FAKELUA_LIBDIR="${BUILD_DIR}/lib64"
    elif [ -f "${BUILD_DIR}/src/libfakelua.so" ]; then
        export FAKELUA_LIBDIR="${BUILD_DIR}/src"
    else
        echo "ERROR: libfakelua.so not found"
        exit 1
    fi
}

# ---- Phase 2: Build fuzz targets with clang + libFuzzer ----
phase2_build() {
    echo ""
    echo "=== Phase 2: Building fuzz targets (clang + libFuzzer) ==="

    if ! command -v clang++ &>/dev/null; then
        echo "ERROR: clang++ required for libFuzzer"
        exit 1
    fi

    find_libs

    local FUZZ_FLAGS="-fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer -g"
    local INC_FLAGS="-I${PROJECT_ROOT}/include -I${PROJECT_ROOT}/src -I${PROJECT_ROOT}/src/platform -I${FUZZ_DIR}"
    # Link against GCC 15's libstdc++ to resolve C++20/C++23 symbols
    local LD_FLAGS="-L${BRIDGE_LIBDIR} -L${FAKELUA_LIBDIR} -L${GCC15_LIBDIR} -L/usr/local/lib"
    local RPATH_FLAGS="-Wl,-rpath,${FAKELUA_LIBDIR} -Wl,-rpath,${GCC15_LIBDIR}"

    mkdir -p "${OUTPUT_DIR}"

    # ---- fuzz_compile ----
    echo "Building fuzz_compile..."
    clang++ ${FUZZ_FLAGS} \
        ${INC_FLAGS} \
        "${FUZZ_DIR}/fuzz_compile.cpp" \
        ${LD_FLAGS} \
        -lfuzz_bridge -lfakelua -lstdc++ \
        ${RPATH_FLAGS} \
        -o "${OUTPUT_DIR}/fuzz_compile"
    echo "  -> ${OUTPUT_DIR}/fuzz_compile"

    # ---- fuzz_differential ----
    echo "Building fuzz_differential..."
    clang++ ${FUZZ_FLAGS} \
        ${INC_FLAGS} \
        -I/usr/local/include \
        "${FUZZ_DIR}/fuzz_differential.cpp" \
        ${LD_FLAGS} \
        -lfuzz_bridge -lfakelua -llua -lstdc++ \
        ${RPATH_FLAGS} \
        -o "${OUTPUT_DIR}/fuzz_differential"
    echo "  -> ${OUTPUT_DIR}/fuzz_differential"

    echo ""
    echo "[OK] Phase 2 complete"
}

# ---- Quick test ----
quick_test() {
    local target="${1:-fuzz_compile}"
    local bin="${OUTPUT_DIR}/${target}"

    if [ ! -x "${bin}" ]; then
        echo "ERROR: ${bin} not found"
        exit 1
    fi

    echo ""
    echo "=== Quick test: ${target} (1000 runs) ==="
    echo ""

    if [ -d "${CORPUS_DIR}" ]; then
        "${bin}" "${CORPUS_DIR}" -max_len=4096 -runs=1000 2>&1
    else
        "${bin}" -max_len=4096 -runs=1000 2>&1
    fi

    echo ""
    echo "[OK] Quick test passed"
}

# ---- Clean ----
do_clean() {
    echo "Cleaning build_fuzz..."
    rm -rf "${BUILD_DIR}"
    echo "[OK] Cleaned"
}

# ---- Main ----
main() {
    case "${1:-build}" in
        clean)
            do_clean
            ;;
        test)
            phase1_build
            phase2_build
            quick_test "${2:-fuzz_compile}"
            ;;
        test-diff)
            phase1_build
            phase2_build
            quick_test "fuzz_differential"
            ;;
        build)
            phase1_build
            phase2_build
            ;;
        *)
            echo "Usage: $0 {build|clean|test [target]|test-diff}"
            exit 1
            ;;
    esac

    if [ "${1:-build}" != "clean" ]; then
        echo ""
        echo "Fuzz binaries: ${OUTPUT_DIR}/"
        echo ""
        echo "Long run examples:"
        echo "  ${OUTPUT_DIR}/fuzz_compile -max_len=4096 -runs=1000000"
        echo "  ${OUTPUT_DIR}/fuzz_compile -dict=${FUZZ_DIR}/lua_keywords.dict -jobs=4 -workers=4 -max_len=4096"
    fi
}

main "$@"
