#!/usr/bin/env bash
# Format all Lua files under test/lua with EmmyLua CodeFormat.
#
# Requires: CodeFormat on PATH (build/install EmmyLuaCodeStyle).
# Usage:
#   ./test/lua/format.sh
#   CODEFORMAT=/path/to/CodeFormat ./test/lua/format.sh

set -euo pipefail

LUA_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CODEFORMAT="${CODEFORMAT:-CodeFormat}"

if ! command -v "${CODEFORMAT}" >/dev/null 2>&1; then
    echo "error: '${CODEFORMAT}' not found on PATH." >&2
    echo "Build EmmyLuaCodeStyle and install CodeFormat, or set CODEFORMAT=..." >&2
    exit 1
fi

echo "Using: $(command -v "${CODEFORMAT}")"
echo "Formatting: ${LUA_DIR}"

# -d: auto-detect .editorconfig in the workspace
# Intentional syntax-error fixtures (e.g. exception/test_compile_fail.lua)
# are skipped by CodeFormat and do not fail the batch.
"${CODEFORMAT}" format -w "${LUA_DIR}" -ow -d \
    --ignores-file "${LUA_DIR}/../../.gitignore"

echo "Done."
