
#include "Slider.h"

#define NODE_COUNT 4

Slider slider1;
Slider slider2;
uint8_t slider1Pins[NODE_COUNT] = {PIN_PC0, PIN_PC1, PIN_PC2, PIN_PC3};
uint8_t slider2Pins[NODE_COUNT] = {PIN_PA5, PIN_PA6, PIN_PA7, PIN_PB5};
uint8_t slider1Driver = 12;
uint8_t slider2Driver = 13;


uint8_t DeviceRegisters[8];


void setup() {
  // put your setup code here, to run once:
  slider1.begin(slider1Pins, slider1Driver);
  slider2.begin(slider2Pins, slider2Driver);
  
  Wire.pins(PIN_PB1, PIN_PB0);
  Wire.begin(0x69);
  Wire.onReceive(receiveHandler);
  Wire.onRequest(requestHandler);

}

void loop() {
  // put your main code here, to run repeatedly:
  ptc_process(millis());
  slider1.update();
  slider2.update();

  DeviceRegisters[0x00] = slider1.getPosition();
  DeviceRegisters[0x01] = slider2.getPosition();
  DeviceRegisters[0x02] = slider1.getPositionHysteresis();
  DeviceRegisters[0x03] = slider2.getPositionHysteresis();

  DeviceRegisters[0x04] = slider1.getMagnitude();
  DeviceRegisters[0x05] = slider2.getMagnitude();
  DeviceRegisters[0x06] = slider1.getMagnitudeHysteresis();
  DeviceRegisters[0x07] = slider2.getMagnitudeHysteresis();


}



volatile uint8_t WirePointer = 0;

void receiveHandler(int numbytes) {
  Wire.getBytesRead(); // reset count of bytes read. We don't do anything with it here, but a write is going to reset it to a new value.
  WirePointer = Wire.read() & 0x08; // make sure they can't write off the end of the array!
  numbytes--; // we just read a byte, so we should decrement this.
  while (numbytes > 0) {

    DeviceRegisters[WirePointer] = Wire.read();
    WirePointer++;          // increment the pointer.
    WirePointer &= 0x1F;    // Wrap around if it's gone over 32;
    numbytes--;             // decrement remaining bytes.
  }
}

void requestHandler() {
  uint8_t bytes_read = Wire.getBytesRead();
  WirePointer       += bytes_read;
  WirePointer       &= 0x1F;

  for (byte i = 0; i < 32; i++) {
    Wire.write(DeviceRegisters[(WirePointer + i) & 0x1F]);
  }

}


void ptc_event_callback(const ptc_cb_event_t eventType, cap_sensor_t* node) {
  #ifdef DEBUG_SERIAL
  if (PTC_CB_EVENT_TOUCH_DETECT == eventType) {
    Serial.print("node touched:");
    Serial.println(ptc_get_node_id(node));
  } else if (PTC_CB_EVENT_TOUCH_RELEASE == eventType) {
    Serial.print("node released:");
    Serial.println(ptc_get_node_id(node));
  } else if (PTC_CB_EVENT_CONV_MUTUAL_CMPL == eventType) {
    // Do more complex things here
  } else if (PTC_CB_EVENT_CONV_CALIB & eventType) {
    if (PTC_CB_EVENT_ERR_CALIB_LOW == eventType) {
      Serial.print("Calib error, Cc too low.");
    } else if (PTC_CB_EVENT_ERR_CALIB_HIGH == eventType) {
      Serial.print("Calib error, Cc too high.");
    } else if (PTC_CB_EVENT_ERR_CALIB_TO == eventType) {
      Serial.print("Calib error, calculation timeout.");
    } else {
      Serial.print("Calib Successful.");
    }
    Serial.print(" Node: ");
    Serial.println(ptc_get_node_id(node));
  }
  #endif
}