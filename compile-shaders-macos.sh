#!/bin/bash
mkdir -p shader/build

echo "Compiling vertex shader"
shaderc \
-f shader/v_simple.sc -o shader/build/v_simple.bin \
--platform osx --type vertex --verbose -i ./ -p metal

echo "Compiling fragment shader"
shaderc \
-f shader/f_simple.sc -o shader/build/f_simple.bin \
--platform osx --type fragment --verbose -i ./ -p metal

echo "Compiling compute shader"
shaderc \
-f shader/compute_ray_cast.sc -o shader/build/compute_ray_cast.bin \
--platform osx --type compute --verbose -i ./ -p metal
