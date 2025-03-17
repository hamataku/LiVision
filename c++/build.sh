#!/bin/bash

cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DSUPERBUILD=ON
cmake --build build