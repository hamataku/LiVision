#!/bin/bash
set -euo pipefail

# shaderc のパスは引数で受け取る
SHADERC=${1:-shaderc}

mkdir -p shader/bin

# 実行環境に応じて platform/type を設定
OS=$(uname)
case "$OS" in
    Linux)
        PLATFORM="linux"
        TYPE="spirv"
        SUFFIX="_linux"
        ;;
    Darwin)
        PLATFORM="osx"
        TYPE="metal"
        SUFFIX="_mac"
        ;;
    MINGW*|MSYS*|CYGWIN*|Windows_NT)
        PLATFORM="windows"
        TYPE="d3d11"
        SUFFIX="_win"
        ;;
    *)
        echo "Unknown OS: $OS"
        exit 1
        ;;
esac

# ヘルパー関数
compile_shader() {
    local SRC=$1
    local OUT_BASE=$2
    local SHADER_TYPE=$3
    OUT="${OUT_BASE}${SUFFIX}.bin"
    echo "Compiling $SHADER_TYPE shader: $SRC -> $OUT"
    "$SHADERC" \
        -f "$SRC" -o "$OUT" \
        --platform "$PLATFORM" --type "$SHADER_TYPE" --verbose -i ./ -p "$TYPE"
}

# shaders
compile_shader shader/v_simple.sc shader/bin/v_simple vertex
compile_shader shader/f_simple.sc shader/bin/f_simple fragment
compile_shader shader/v_points.sc shader/bin/v_points vertex
compile_shader shader/f_points.sc shader/bin/f_points fragment
