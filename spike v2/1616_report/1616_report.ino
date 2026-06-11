
#include <Wire.h>
#include <ptc.h>
#include "TinyTouch.h"

#define NODE_COUNT 12
#define CUTOFF 127

//TinyTouch touch;


uint8_t sensors_pins[NODE_COUNT] = {PIN_PC3, PIN_PC2, PIN_PC1, PIN_PC0, PIN_PB0, PIN_PB1, PIN_PB5, PIN_PB4, PIN_PA6, PIN_PA7, PIN_PA5, PIN_PA4};

//uint8_t sensors_pins[NODE_COUNT] = {5, 6, 7, 8, 9, 10, 13, 14, 15, 16, 17, 18};

uint8_t id_to_index[NODE_COUNT + 2] = {0, 1, 2, 3, 4, 5, 255, 255, 6, 7, 8, 9, 10, 11};

int16_t zeros[12] = {0x029c, 0x0332, 0x03f9, 0x03f8, 0x01f0, 0x01f8, 0x0319, 0x03f8, 0x03f8, 0x03a6, 0x01f0, 0x01f8};

bool values[12];


//cap_sensor_t s_nodes[12];

int16_t iv[12];

void setup() {
  touch.begin(sensors_pins, sizeof(sensors_pins));


  for (uint8_t p = 0; p < 12; p++) {
    //ptc_add_selfcap_node(&s_nodes[p], 0, PIN_TO_PTC(sensors_pins[p]));
    //ptc_node_set_charge_share_delay(&s_nodes[p], 0);

    //ptc_node_set_prescaler(&s_nodes[p], PTC_PRESC_DIV64_gc);

    delay(200);
  }

  for (uint8_t p = 0; p < 12; p++) {
    ptc_node_set_gain(&s_nodes[p], PTC_GAIN_32);

    //delay(200);
  }


  Wire.pins(20, 1);  
  //Wire.pins(PIN_PB1, PIN_PB0);
  //Wire.pins(PIN_PA1, PIN_PA2);
  
  Wire.begin(0x61);
  Wire.onRequest(requestHandler);

}

void loop() {
  //ptc_process(millis());

  touch.touchHandle();

  for (uint8_t p = 0; p < 12; p++){
    //iv[p] = ptc_get_node_sensor_value(&s_nodes[p]) - 512;
    iv[p] = touch.getValue(p);
  }
}


void requestHandler() {
  for (uint8_t r = 0; r < 12; r++){
    //Wire.write(values[r]);
    Wire.write((uint8_t) (iv[r] >> 8) & 0xFF);
    Wire.write((uint8_t) (iv[r]) & 0xFF);
  }
}


/*
// callback that is called by ptc_process at different points to ease user interaction
void ptc_event_callback(const ptc_cb_event_t eventType, cap_sensor_t* node) {
  if (PTC_CB_EVENT_TOUCH_DETECT == eventType) {
    //values[id_to_index[ptc_get_node_id(node)]] = true;

  } else if (PTC_CB_EVENT_TOUCH_RELEASE == eventType) {
    //values[id_to_index[ptc_get_node_id(node)]] = false;

  } else if (PTC_CB_EVENT_CONV_SELF_CMPL == eventType) {
    // Do more complex things here
  } else if (PTC_CB_EVENT_CONV_CALIB & eventType) {
    if (PTC_CB_EVENT_ERR_CALIB_LOW == eventType) {
      //MySerial.print("Calib error, Cc too low.");
    } else if (PTC_CB_EVENT_ERR_CALIB_HIGH == eventType) {
      //MySerial.print("Calib error, Cc too high.");
    } else if (PTC_CB_EVENT_ERR_CALIB_TO == eventType) {
      //MySerial.print("Calib error, calculation timeout.");
    } else {
      //MySerial.print("Calib Successful.");
    }
    //MySerial.print(" Node: ");
    //MySerial.println(ptc_get_node_id(node));
    iv[id_to_index[ptc_get_node_id(node)]] = ptc_get_node_sensor_value(node);
  }
}

*/
