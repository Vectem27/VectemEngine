#!/usr/bin/env bash
set -e

cd "$(dirname "$0")"
mkdir -p generated

XML_DIR=deps/wayland
OUTPUT_DIR=generated

for xml in "$XML_DIR"/*.xml; do
    [ -e "$xml" ] || continue
    proto="$(basename "$xml" .xml)"

    wayland-scanner client-header \
        "$xml" \
        "$OUTPUT_DIR/$proto-client-protocol.h"

    wayland-scanner private-code \
        "$xml" \
        "$OUTPUT_DIR/$proto-protocol.c"
done

echo "Wayland file generation done"
