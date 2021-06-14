#!/bin/sh

if [ ! -d run ]; then
  mkdir run
fi

#clone bluez if not cloned
cd run
if [ ! -d bluez ]; then 
  git clone git@github.com:bluez/bluez.git 
fi 

#build btvirt tool if it doesnt exist
if [ ! -f bluez/emulator/btvirt ]; then
  cd bluez 
  ./bootstrap
  ./configure --prefix=/usr --mandir=/usr/share/man --sysconfdir=/etc --localstatedir=/var --enable-experimental --enable-testing
  make 

  cd ..
fi

#create two LE controllers
sudo ./bluez/emulator/btvirt -L -l2 