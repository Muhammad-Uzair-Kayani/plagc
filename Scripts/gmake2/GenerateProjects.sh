#!/usr/bin/env bash
set -euo pipefail

# Generate GNU Make project files for Linux / VS Code.
# Run from any directory. Requires a Linux Premake 5 executable.

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd -- "$SCRIPT_DIR/../.." && pwd)"

if [[ -n "${PREMAKE:-}" ]]; then
    PREMAKE_BIN="$PREMAKE"
elif [[ -x "$ROOT_DIR/third-party/premake/premake5" ]]; then
    PREMAKE_BIN="$ROOT_DIR/third-party/premake/premake5"
elif command -v premake5 >/dev/null 2>&1; then
    PREMAKE_BIN="$(command -v premake5)"
else
    echo "[ERROR] Linux Premake 5 executable not found." >&2
    echo "Install premake5 on PATH, place it at third-party/premake/premake5," >&2
    echo "or invoke: PREMAKE=/path/to/premake5 bash Scripts/gmake2/GenerateProjects.sh" >&2
    exit 1
fi

cd "$ROOT_DIR"
"$PREMAKE_BIN" gmake2
