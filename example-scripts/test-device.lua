--setup device
TST_SRV1 = "deadbeee-eeee-eeee-eeee-eeeeeeeeeeef"
TST_CHR1 = "deadbeef-0000-1000-0000-008cafebabe1"
TST_CHR2 = "deadbeef-0000-1000-0000-008cafebabe2"
TST_CHR3 = "deadbeef-0000-1000-0000-008cafebabe3"
TST_DESC1 = "12345678-1234-5678-1234-56789abcdef1"
TST_DESC2 = "12345678-1234-5678-1234-56789abcdef2"
TST_DESC3 = "12345678-1234-5678-1234-56789abcdef3"

dev1 = ble.createDevice ("ble-service-test-device")
srvc1 = ble.createService (TST_SRV1)
char1 = ble.createCharacteristic (TST_CHR1)
char2 = ble.createCharacteristic (TST_CHR2)
char3 = ble.createCharacteristic (TST_CHR3)
desc1 = ble.createDescriptor (TST_DESC1)
desc2 = ble.createDescriptor (TST_DESC2)
desc3 = ble.createDescriptor (TST_DESC3)

dev1:addService (srvc1)
srvc1:addCharacteristic (char1)
srvc1:addCharacteristic (char2)
srvc1:addCharacteristic (char3)
char1:addDescriptor (desc1)
char2:addDescriptor (desc2)
char3:addDescriptor (desc3)

ble.registerDevice (dev1)

dev1:powered (true)
dev1:discoverable (true)

char1_value = 0 

char2_value = {0xDE, 0xAD, 0xBE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEF}
char2:setValue (char2_value)
char3:setValue (true)

function Update() 
  char1:setValue(char1_value)
  char1_value = char1_value + 1
end
