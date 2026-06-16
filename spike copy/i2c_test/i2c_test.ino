

#include <Wire.h>


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }

  Wire.begin();
  Wire.setClock(100000);


}

void loop() {
  // put your main code here, to run repeatedly:
  Wire.requestFrom(0x61, 12);
  uint8_t i = 0;
  while (Wire.available()) {
    //Serial.printf("%02x", (uint8_t)Wire.read());
    //uint16_t val = Wire.read() << 8;
    //val += Wire.read();
    uint8_t val = Wire.read();
    //Serial.printf("%07d", static_cast<int16_t>(val));
    if (i<12){
      Serial.print(" val");
      Serial.print(i);
      Serial.print(":");
      Serial.printf("%03d", val);
      Serial.print(",");
    }
    i++;
  }
  Serial.println();
}
