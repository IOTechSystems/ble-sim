#!/bin/sh

set -e -x
MNAME=`uname -m`

case $MNAME in
  aarch64)
    ARCH=arm64
    ;;
  armv7l)
    ARCH=arm
    ;;
  i686)
    ARCH=x86
    ;;
  *)
    ARCH=$MNAME
    ;;
esac

export ARCH
export VER=`cat ./VERSION`
export EXP=iotechsys
export SHA=`git rev-parse HEAD`
