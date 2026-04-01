#!/bin/bash

set -e

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT_DIR"

"$ROOT_DIR/../VectemBuildTool/bin/VectemBuildTool" . VectemEngine Engine Debug

echo "Compilation finished."