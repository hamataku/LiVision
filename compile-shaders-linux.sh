#!/bin/bash

# compile shaders

mkdir -p shader/build

echo "Compiling vertex shader"
./third-party/build/bin/shaderc \
-f shader/v_simple.sc -o shader/build/v_simple.bin \
--platform linux --type vertex --verbose -i ./ -p spirv

echo "Compiling fragment shader"
./third-party/build/bin/shaderc \
-f shader/f_simple.sc -o shader/build/f_simple.bin \
--platform linux --type fragment --verbose -i ./ -p spirv

echo "Compiling compute shader"
./third-party/build/bin/shaderc \
-f shader/compute_ray_cast.sc -o shader/build/compute_ray_cast.bin \
--platform linux --type compute --verbose -i ./ -p spirv