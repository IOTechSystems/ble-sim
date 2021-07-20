BLUEZ_VERSION=5.60

wget http://www.kernel.org/pub/linux/bluetooth/bluez-$BLUEZ_VERSION.tar.xz -O bluez.tar.xz
tar -xvf bluez.tar.xz && mv bluez-$BLUEZ_VERSION bluez
rm -r bluez.tar.xz