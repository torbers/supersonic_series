
#include "Slider.h"

#define NODE_COUNT 4

Slider slider1;
Slider slider2;
uint8_t slider1Pins[NODE_COUNT] = {PIN_PB5, PIN_PB4, PIN_PB1, PIN_PB0};
uint8_t slider2Pins[NODE_COUNT] = {PIN_PA7, PIN_PA6, PIN_PA5, PIN_PA4};
uint8_t slider1Driver = 12;
uint8_t slider2Driver = 13;


uint8_t DeviceRegisters[10];


void setup() {
  // put your setup code here, to run once:
  slider1.begin(slider1Pins, slider1Driver);
  slider2.begin(slider2Pins, slider2Driver);
  
  Wire.pins(PIN_PA1, PIN_PA2);
  Wire.begin(0x62);
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

  DeviceRegisters[0x08] = slider1.isTouched();
  DeviceRegisters[0x09] = slider2.isTouched();


}

void requestHandler() {
  for (uint8_t r = 0; r < 10; r++){
    Wire.write(DeviceRegisters[r]);
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