
TST_SRVC1 = "0000180d-0000-1000-8000-00805f9b34fb"
TST_CHR1 = "00002a38-0000-1000-8000-00805f9b34fb"
TST_CHR2 = "00002a39-0000-1000-8000-00805f9b34fb"
TST_DESC1 = "12345678-1234-5678-1234-56789abcdef2"

dev1 = ble.createDevice ("lua-test-dev-1");
srvc1 = ble.createService (TST_SRVC1, true);
char1 = ble.createCharacteristic (TST_CHR1);
desc1 = ble.createDescriptor (TST_DESC1);

dev1:addService(srvc1);
srvc1:addCharacteristic(char1);
char1:addDescriptor(desc1);

ble.registerDevice (dev1);
