#!/bin/sh
. ./scripts/env.sh

docker rmi $EXP/ble-sim:$VER-$ARCH
