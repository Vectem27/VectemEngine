#!/bin/bash

set -e

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT_DIR"

"$ROOT_DIR/Build/Linux/Debug/bin/VectemEngine" "$@"