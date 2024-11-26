#include <Wire.h>

void setup() {
  Serial.begin(9600); 
  Wire.begin();         
  Serial.println("Scanning for I2C devices...");
}

void loop() {
  int devicesFound = 0;

  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print("I2C device found at address 0x");
      Serial.println(address, HEX);
      devicesFound++;
    }
  }

  if (devicesFound == 0) {
    Serial.println("No I2C devices found.");
  } else {
    Serial.print("Total I2C devices found: ");
    Serial.println(devicesFound);
  }

  delay(5000);  =
}
