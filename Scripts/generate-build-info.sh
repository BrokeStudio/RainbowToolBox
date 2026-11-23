#!/bin/sh

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
APP_DIR=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
COMMON_DIR="$APP_DIR/Common/generated"
HEADER_FILE="$COMMON_DIR/build_info.h"
TEMP_FILE="$HEADER_FILE.tmp"

GIT_HASH=$(git -C "$APP_DIR" rev-parse --short=8 HEAD 2>/dev/null)

if [ -z "$GIT_HASH" ]; then
    APP_BUILD_ID="unknown"
else
    DIRTY_SUFFIX=""

    if [ -n "$(git -C "$APP_DIR" status --porcelain)" ]; then
        DIRTY_SUFFIX=".dirty"
    fi

    APP_BUILD_ID="g${GIT_HASH}${DIRTY_SUFFIX}"
fi

mkdir -p "$COMMON_DIR"
printf '#pragma once\n\n#define APP_BUILD_ID "%s"\n' "$APP_BUILD_ID" > "$TEMP_FILE"

if [ -f "$HEADER_FILE" ] && cmp -s "$TEMP_FILE" "$HEADER_FILE"; then
    rm "$TEMP_FILE"
else
    mv "$TEMP_FILE" "$HEADER_FILE"
fi
