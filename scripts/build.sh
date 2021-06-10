#!/bin/sh
set -e -x

# Find root directory and system type
ROOT=$(dirname $(dirname $(readlink -f $0)))

cd $ROOT

mkdir -p $ROOT/build/release/ble-sim
cd $ROOT/build/release/ble-sim

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release $ROOT
make 2>&1 | tee release.log

# Cmake debug build
mkdir -p $ROOT/build/debug/ble-sim
cd $ROOT/build/debug/ble-sim
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug $ROOT
make 2>&1 | tee debug.log