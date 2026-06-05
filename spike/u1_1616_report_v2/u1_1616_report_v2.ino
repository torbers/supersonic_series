#include "Slider.h"

#define NODE_COUNT 12
Slider slider1;

uint8_t sensors_pins[NODE_COUNT] = {PIN_PC3, PIN_PC2, PIN_PC1, PIN_PC0, PIN_PB0, PIN_PB1, PIN_PB5, PIN_PB4, PIN_PA6, PIN_PA7, PIN_PA5, PIN_PA4};
volatile int16_t iv[12];

int16_t max_vals[NODE_COUNT] = {512, 110, 510, 402, 336, 292, 508, 508, 512, 500, 68, 512};

uint8_t cal_max[NODE_COUNT];


void setup() {
  slider1.begin(sensors_pins);
  delay(100);

  slider1.update();
  delay(100);

  for (uint8_t i = 0; i < NODE_COUNT; i++){
    //ptc_node_set_gain(&slider1.s_nodes[i], 1, 1); 
    //delay(100);
  }

  /*
  ptc_node_set_gain(&slider1.s_nodes[0], 1, 1); 
  ptc_node_set_gain(&slider1.s_nodes[1], 1, 1); 
  ptc_node_set_gain(&slider1.s_nodes[2], 1, 1); 
  ptc_node_set_gain(&slider1.s_nodes[3], 1, 1); 
  */

  // put your setup code here, to run once:
  //Wire.pins(PIN_PB1, PIN_PB0);
  Wire.pins(PIN_PA1, PIN_PA2);
  
  Wire.begin(0x61);
  Wire.onRequest(requestHandler);
}

void loop() {
  // put your main code here, to run repeatedly:
  ptc_process(millis());
  slider1.update();
  slider1.capTouchCurrent[0] -= 504;

  for (uint8_t p = 0; p < 12; p++){
    iv[p] = slider1.capTouchCurrent[p];
    cal_max[p] = (uint8_t) constrain((float(iv[p]) / float(max_vals[p]) * 255), 0, 255);
    //iv[p] = slider1.capTouchRead(p);
  }

}

void requestHandler() {
  for (uint8_t r = 0; r < 12; r++){
    //Wire.write(values[r]);
    //Wire.write((uint8_t) (iv[r] >> 8) & 0xFF);
    //Wire.write((uint8_t) (iv[r]) & 0xFF);
    Wire.write(cal_max[r]);
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