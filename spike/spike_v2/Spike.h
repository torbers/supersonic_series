
#include <Adafruit_MPR121.h>
#include <Wire.h>
#include <Arduino.h>


#define MPR0_ADDR 0x5A
#define MPR1_ADDR 0x5B
#define U2_ADDR 0x62

#define NUM_SENSORS 12

#define TOUCH_CUTOFF 127

class Spike {
  public:
    // sensor chips objects
    Adafruit_MPR121 mpr0 = Adafruit_MPR121();
    Adafruit_MPR121 mpr1 = Adafruit_MPR121();

    uint8_t begin();
    uint8_t update();

    // bitmaps of touched pins
    uint16_t mpr0_touched;
    uint16_t mpr1_touched;
    uint16_t mpr0_old;
    uint16_t mpr1_old;

    // induv pads
    bool plus_touched;  // momentary
    bool minus_touched;

    bool pad_a;  // constant
    bool pad_b;
    bool pad_c;

    bool pad_n;  // constant
    bool pad_w;
    bool pad_s;
    bool pad_e;
    bool pad_center;

    uint8_t event_note;
    uint8_t note;
    bool note_touched;
    bool any_note;
    bool all_off_now;

    //uint8_t* getChord();
    //uint8_t getABC();
    int8_t getPM();

    uint8_t getNote();

    uint8_t slider_pos_l;
    uint8_t slider_pos_r;
    bool slider_touched_l;
    bool slider_touched_r;


    uint8_t current_chord[2];
    uint8_t chord_index_arr[5] = {1, 3, 0, 7, 5};
    uint8_t chord_index = 0; // start at center, then counter clockwise from N

};


uint8_t Spike::begin() {
  Wire.begin();
  Wire.setClock(100000);

  if (!mpr0.begin(MPR0_ADDR, &Wire)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 0 found!");

  mpr0.setAutoconfig(true);

  if (!mpr1.begin(MPR1_ADDR, &Wire)) {
    Serial.println("MPR121 1 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 1 found!");

  mpr1.setAutoconfig(true);

  return 0;
}


uint8_t Spike::update() {
  mpr0_old = mpr0_touched;
  mpr1_old = mpr1_touched;

  mpr0_touched = mpr0.touched();
  mpr1_touched = mpr1.touched();

  // bitmaps of newly touched or released pins
  uint16_t events0 = ~ (mpr0_touched & mpr0_old);
  uint16_t events1 = ~ (mpr1_touched & mpr1_old);

  uint16_t new_touched0 = events0 & mpr0_touched;
  uint16_t new_touched1 = events1 & mpr1_touched;
  uint16_t new_released0 = events0 & mpr0_old;
  uint16_t new_released1 = events1 & mpr1_old;

  // plus, minus, abc
  plus_touched = new_touched0 & 0b100;
  minus_touched = new_touched0 & 0b1000;

  pad_a = mpr0_touched & 0b1000000;
  pad_b = mpr0_touched & 0b10000;
  pad_c = mpr0_touched & 0b100000;

  pad_n = mpr0_touched & 0b100000000000;
  pad_w = mpr0_touched & 0b10000000000;
  pad_s = mpr0_touched & 0b10000000;
  pad_e = mpr0_touched & 0b1000000000;
  pad_center = mpr0_touched & 0b1000000000;

  pad_e = pad_e || pad_center;

  // note keys
  note_touched = true;
  if      (new_touched1 & 0b100000000000) {event_note = 0;}
  else if (new_touched1 & 0b010000000000) {event_note = 1;}
  else if (new_touched1 & 0b001000000000) {event_note = 2;}
  else if (new_touched1 & 0b000100000000) {event_note = 3;}
  else if (new_touched1 & 0b000010000000) {event_note = 4;}
  else if (new_touched1 & 0b000001000000) {event_note = 5;}
  else if (new_touched1 & 0b000000100000) {event_note = 7;}
  else if (new_touched1 & 0b000000010000) {event_note = 6;}
  else if (new_touched1 & 0b000000001000) {event_note = 9;}
  else if (new_touched1 & 0b000000000100) {event_note = 8;}
  else if (new_touched1 & 0b000000000010) {event_note = 10;}
  else if (new_touched1 & 0b000000000001) {event_note = 11;}
  else {note_touched = false;}

  note = event_note;
  all_off_now = (!mpr1_touched) && new_released1;


  // 1616 sliders
  Wire.requestFrom(U2_ADDR, 10);
  uint8_t r = 0;
    while (Wire.available()) {
      uint8_t read = Wire.read();
      //Serial.printf("%02x ", read);
      if (r == 0) {slider_pos_l = read;}
      if (r == 1) {slider_pos_r = read;}
      if (r == 8) {slider_touched_l = read;}
      if (r == 9) {slider_touched_r = read;}
      r++;
  }
  //Serial.println();
  return 0;

}

/*

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
    chord_index = 0;
  }

  if (u0_read[ind2] > TOUCH_CUTOFF){
    current_chord[1] = ind2;
  }
  else {
    current_chord[1] = 0xFF;
  }

  if (current_chord[0] != 0xFF) {
    chord_index = chord_index_arr[current_chord[0]];

    if (current_chord[1] != 0xFF) {;}
  }

  return current_chord;
}

*/

int8_t Spike::getPM() {
  return (plus_touched - minus_touched);
}


uint8_t Spike::getNote() {
  return note;
}

