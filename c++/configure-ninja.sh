#!/bin/bash

cmake -B build/release-ninja -G Ninja -DCMAKE_BUILD_TYPE=Release -DSUPERBUILD=ON
