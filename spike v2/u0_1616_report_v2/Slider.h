
#include "Hysteresis.h"
#include <Wire.h>
#include <ptc_touch.h>

#define NODE_COUNT 12

class Slider {
  public:
    //Slider();
    uint8_t begin(uint8_t *sensors);
    int16_t capTouchRead(uint8_t pin);
    int16_t getPosition(int range=0xFF);
    int16_t getMaxReading();
    int16_t getMagnitude();
    uint8_t getPositionHysteresis();
    uint8_t getMagnitudeHysteresis();
    
    uint32_t duration();
    bool update();
    bool isTouched();
    
    static const uint8_t numSensors = NODE_COUNT;
    uint8_t capTouchPins[numSensors] = {};
    uint8_t driver_pin = 0;

    volatile int16_t capTouchCal[numSensors];
    volatile int16_t capTouchCurrent[numSensors];
    volatile uint8_t overCalCount[numSensors];

    int16_t currentMaxReading= 0;
    uint8_t primary = 0;
    bool isTouch = 0;
    uint8_t initialTouchPos = 0;
    uint8_t finalTouchPos = 0;
    uint8_t currentTouchedPositions = 0;
    uint8_t allTouchedPositions = 0;
    uint32_t touchTimer = 0;

    cap_sensor_t s_nodes[numSensors];

    Hysteresis <int16_t> positionHysteresis = Hysteresis <int16_t> (4);
    Hysteresis <int16_t> magnitudeHysteresis = Hysteresis <int16_t> (4);
};


// Slider class

uint8_t Slider::begin(uint8_t sensors[numSensors]) {
  
  for (uint8_t p = 0; p < numSensors; p++) {
    Serial.print("Sensor no.: ");
    Serial.println(p);
    capTouchPins[p] = sensors[p];
    Serial.print("goes to pin no.: ");
    Serial.println(capTouchPins[p]);
  }

  for (uint8_t pin = 0; pin < numSensors; pin++) {
    ptc_add_selfcap_node(&s_nodes[pin], PIN_TO_PTC(capTouchPins[pin]), 0);
    delay(100);
    //ptc_node_set_gain(&s_nodes[pin], 0x03, 0x03);
  }

  ptc_process(millis());
  capTouchRead(0);

  for (uint8_t pin = 0; pin < numSensors; pin++) {
    ptc_process(millis());
    capTouchCal[pin] = capTouchRead(pin);
    overCalCount[pin] = 0;
    delay(100);
  }

  for (uint8_t p = 0; p < numSensors; p++) {
    Serial.print("Sensor read: ");
    Serial.println(capTouchCal[p]);
  }
  
  return 0;
}

int16_t Slider::capTouchRead(uint8_t pin) {
  //return ptc_get_node_sensor_value(&s_nodes[capTouchPins[pin] - 1]);
  return ptc_get_node_sensor_value(&s_nodes[pin]);// - 512;
}

int16_t Slider::getPosition(int range) {
  float position = primary;
  if (primary < numSensors-1)
    position += (pow((float)capTouchCurrent[primary + 1] / (float)getMaxReading(),0.45)) / 2.0;
  if (primary > 0)
    position -= (pow((float)capTouchCurrent[primary - 1] / (float)getMaxReading(),0.45)) / 2.0;
  
  position = position * (float)range / float(numSensors-1);
  return position;
}

int16_t Slider::getMaxReading() {
  return currentMaxReading;
}

int16_t Slider::getMagnitude() {
  uint8_t j;
  long magnitude = 0;
  for (j = 0; j < NODE_COUNT; j++) {
    magnitude += pow(capTouchCurrent[j], 2);
  }
  return sqrt(magnitude);
}

uint8_t Slider::getPositionHysteresis() {
  Serial.println(getPosition());
  int16_t ph = positionHysteresis.add(getPosition());
  Serial.println(ph);
  return ph;
}

uint8_t Slider::getMagnitudeHysteresis() {
  return magnitudeHysteresis.add(getMagnitude());
}

bool Slider::update() {
  uint8_t j;
  currentMaxReading=0;
  for (j = 0; j < numSensors; j++) {
    int val = capTouchRead(j);
    if (val < capTouchCal[j]) {
      capTouchCal[j] = val;
    } 
    else if (val > (capTouchCal[j] + 1)) {
      overCalCount[j]++;
      if (overCalCount[j] > 3) {
        //overCalCount[j] = 0;
        //capTouchCal[j] += 1;
      } 
      else {
        //overCalCount[j]=0;
      }
    }
    capTouchCurrent[j] = val - capTouchCal[j];  
    if (capTouchCurrent[j] > getMaxReading()) {
      currentMaxReading = capTouchCurrent[j];
      primary = j;
    }
  }

  bool completedTouch = false;

  if (currentMaxReading > 255) {
    if (!isTouch) {
      isTouch = true;
      touchTimer = millis();
      initialTouchPos = primary;
      finalTouchPos = primary;
      currentTouchedPositions = 0;
      allTouchedPositions = 0;
    } else {
      finalTouchPos = primary;
      currentTouchedPositions = 0;
      for (j = 0; j < numSensors; j++) {
        if (capTouchCurrent[j] > 64)
          currentTouchedPositions |= (1 << j);
        allTouchedPositions |= (1 << j);
      }
    }
  } else {
    if (isTouch) {
      isTouch = false;
      touchTimer = millis() - touchTimer;
      completedTouch = true;
    }
  }

  return completedTouch;
}

bool Slider::isTouched() {
  return isTouch;
}

uint32_t Slider::duration() {
  if (isTouch) {
    return millis() - touchTimer;
  }
  return touchTimer;
}