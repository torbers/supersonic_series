
#include <Wire.h>
#include <Arduino.h>


#define U0_ADDR 0x60
#define U1_ADDR 0x61
#define U2_ADDR 0x62

#define NUM_SENSORS 12

#define TOUCH_CUTOFF 127

class Spike {
  public:
    uint8_t begin();
    uint8_t update();

    uint8_t* getChord();
    uint8_t getABC();
    uint8_t getPM();

    uint8_t getNote();



    uint8_t u0_read[NUM_SENSORS];
    uint8_t u1_read[NUM_SENSORS];

    uint8_t current_chord[2];

};


uint8_t Spike::begin() {
  Wire.begin();
  Wire.setClock(100000);

  return 0;
}


uint8_t Spike::update() {
  Wire.requestFrom(U0_ADDR, NUM_SENSORS);
  uint8_t i = 0;
  while (Wire.available()) {
    u0_read[i] = Wire.read();
    i++;
  }

  Wire.requestFrom(U1_ADDR, NUM_SENSORS);
  i = 0;
  while (Wire.available()) {
    u1_read[i] = Wire.read();
    i++;
  }


  // highest 2 of 0..4 on U0 or None  // Chord sensors
  getChord();

  // highest of 5..7 on U0 or None  // ABC sensors
  
  // highest of 8 (needs hysterisis) or 9 on U0 or None  // +- sensors

  // highest value of 0..11 on U1 or None  // Note sensors

  return 0;

}

uint8_t* Spike::getChord() {
  // Set and return current_chord, the highest- and second-highest- touched chord sensors.
  // Return 0xFF on one or both if only one or no sensor is touched

  uint8_t max;
  uint8_t ind;
  uint8_t ind2;

  for (uint8_t s = 0; s <= 4; s++) {
    if (max <= u0_read[s]) {
      ind2 = ind;
      ind = s;
      max = u0_read[s];
    }
  }


  // Just return 0xFF for sensor nos. if sensor is untouched

  if (u0_read[ind] > TOUCH_CUTOFF){
    current_chord[0] = ind;
  }
  else {
    current_chord[0] = 0xFF;
  }

  if (u0_read[ind2] > TOUCH_CUTOFF){
    current_chord[1] = ind2;
  }
  else {
    current_chord[1] = 0xFF;
  }

  return current_chord;
}


uint8_t Spike::getABC() {
  // Get the highest-touched ABC sensor; return 0xFF if none are touched.

  uint8_t max;
  uint8_t ind;

  for (uint8_t s = 5; s <= 7; s++) {
    if (max <= u0_read[s]) {
      ind = s;
      max = u0_read[s];
    }
  }

  // 0xFF = no touch, 0x00..0x02 = highest touched sensor
  if (max > TOUCH_CUTOFF) {
    return ind - 0x05;
  }
  else {
    return 0xFF;
  }
}


uint8_t Spike::getPM() {

  return 0x00;
}


uint8_t Spike::getNote() {
  // Get the highest touched note sensor number
  uint8_t max = 0;
  uint8_t ind;

  for (uint8_t s = 0; s < NUM_SENSORS; s++) {
    if (max <= u1_read[s]) {
      ind = s;
      max = u1_read[s];
    }
  }

  if (max > TOUCH_CUTOFF) {
    return ind;
  }
  else {
    return 0xFF;
  }
}



