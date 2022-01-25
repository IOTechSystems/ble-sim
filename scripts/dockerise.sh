#!/bin/sh
. ./scripts/env.sh

ARCHIVE=$1

docker build --no-cache --tag $EXP/ble-sim:$VER-$ARCH --build-arg BUILD_VER=$VER --build-arg BUILD_SHA=$SHA --file scripts/Dockerfile.ble-sim .

if [ "$ARCHIVE" = true ]; then
  docker push $EXP/ble-sim:$VER-$ARCH
fi
