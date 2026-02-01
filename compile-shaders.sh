#!/bin/bash
set -euo pipefail
mkdir -p shader/build

OS=$(uname)

if [ "$OS" = "Darwin" ]; then
    echo "MacOS"
    PLATFORM="osx"
    TYPE="metal"
elif [ "$OS" = "Linux" ]; then
    echo "Linux"
    PLATFORM="linux"
    TYPE="spirv"
else
    echo "Unknown OS: $OS"
    echo "Please run this script on MacOS or Linux"
fi

echo "Compiling vertex shader"
shaderc \
-f shader/v_simple.sc -o shader/build/v_simple.bin \
--platform $PLATFORM --type vertex --verbose -i ./ -p $TYPE

echo "Compiling fragment shader"
shaderc \
-f shader/f_simple.sc -o shader/build/f_simple.bin \
--platform $PLATFORM --type fragment --verbose -i ./ -p $TYPE

echo "Compiling vertex shader (points)"
shaderc \
-f shader/v_points.sc -o shader/build/v_points.bin \
--platform $PLATFORM --type vertex --verbose -i ./ -p $TYPE

echo "Compiling fragment shader (points)"
shaderc \
-f shader/f_points.sc -o shader/build/f_points.bin \
--platform $PLATFORM --type fragment --verbose -i ./ -p $TYPE

echo "Compiling compute shader"
shaderc \
-f shader/compute_ray_cast.sc -o shader/build/compute_ray_cast.bin \
--platform $PLATFORM --type compute --verbose -i ./ -p $TYPE
