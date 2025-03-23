#!/bin/bash
mkdir -p shader/build

SHADERC_PATH="./generated/bin/shaderc"

echo "Compiling vertex shader"
$SHADERC_PATH \
-f shader/v_simple.sc -o shader/build/v_simple.bin \
--platform osx --type vertex --verbose -i ./ -p metal

echo "Compiling fragment shader"
$SHADERC_PATH \
-f shader/f_simple.sc -o shader/build/f_simple.bin \
--platform osx --type fragment --verbose -i ./ -p metal

echo "Compiling compute shader"
$SHADERC_PATH \
-f shader/compute_ray_cast.sc -o shader/build/compute_ray_cast.bin \
--platform osx --type compute --verbose -i ./ -p metal
