#!/bin/sh

record_sim_device ()
{
  echo "Recording device $1 with controller 00:AA:01:00:00:23"
  MAC_ADDRESS=$1

  DEVICE_DIR=/var/lib/bluetooth/00:AA:01:00:00:23/$MAC_ADDRESS/
  if [ ! -d $DEVICE_DIR ]; then
    mkdir -p $DEVICE_DIR
  fi 
  printf "[General]\nName=simulated_device\nAddressType=public\nSupportedTechnologies=LE;\nTrusted=false\nBlocked=false" > /var/lib/bluetooth/00:AA:01:00:00:23/$MAC_ADDRESS/info
}

record_sim_devices ()
{
  #address generation from bluez source code (btdev.c:
  # bdaddr[0] = id & 0xff;
	# bdaddr[1] = id >> 8;
	# bdaddr[2] = index;
	# bdaddr[3] = 0x01;
	# bdaddr[4] = 0xaa;
	# bdaddr[5] = 0x00;
  count=$(($1>15?15:$1)) #number of devices to record with bluez capped out at 15  (min($1,15))
  echo "Recording $count device(s) with bluez"
  id=0x24 #id is set in the bluez source
  i=1 
  
  while [ $i -le $count ];do

    b0=$(( $id & 0xff ))
    b1=$(( $id >> 8 ))
    
    address=$(printf "00:AA:01:%02X:%02X:%02X\n" $i $b1 $b0)
    record_sim_device $address

    i=$(( i + 1 ))
    id=$(( id + 1 ))
  done

}

if [ "$RUN_BLUEZ" == "true" ];then
  #we need to write bluetooth.conf to the host machine so that dbus has permissions to own the bluez service
  cp /usr/share/dbus-1/system.d/bluetooth.conf /etc/dbus-1/system.d/bluetooth.conf 

  rm -rf /var/lib/bluetooth/* #clear the current cache
  record_sim_devices $DEVICE_COUNT
  echo "Running simulator with bluez in container"
  nsenter --net=/rootns/net /usr/lib/bluetooth/bluetoothd &
  sleep 1
else
  echo "Running simulator with host bluez"
fi 

echo "Running ble sim"
./ble-sim/ble-sim $@
