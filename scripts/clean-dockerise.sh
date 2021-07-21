#!/bin/sh
. ./scripts/env.sh

docker rmi $EXP/bacnet-server:$VER-$ARCH
