
#define MOZZI_CONTROL_RATE 256
#define MOZZI_AUDIO_RATE 16384

#include <Mozzi.h>
#include <Oscil.h>
#include <tables/saw512_int8.h>
#include <ResonantFilter.h>
#include <mozzi_midi.h> // for mtof
#include <AudioDelayFeedback.h>

#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <MIDI.h>
#include <Wire.h>


//#include "Slider.h"

// Settings macros
// Filter
#define LOWPASS 0x00
#define BANDPASS 0x01
#define HIGHPASS 0x02
#define LOWPASS_TRACK 0x03
#define BANDPASS_TRACK 0x04
#define HIGHPASS_TRACK 0x05
#define FILTER_MODES_COUNT 0x06

#define A 0b001
#define B 0b100
#define C 0b010

#define PLUS 0b10
#define MINUS 0b01

#define MAX_OCTAVE 10


Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);



// Audio objects

Oscil <SAW512_NUM_CELLS, MOZZI_AUDIO_RATE> saw0(SAW512_DATA);
Oscil <SAW512_NUM_CELLS, MOZZI_AUDIO_RATE> saw1(SAW512_DATA);
Oscil <SAW512_NUM_CELLS, MOZZI_AUDIO_RATE> saw2(SAW512_DATA);
Oscil <SAW512_NUM_CELLS, MOZZI_AUDIO_RATE> saw3(SAW512_DATA);
Oscil <SAW512_NUM_CELLS, MOZZI_AUDIO_RATE> saw4(SAW512_DATA);
Oscil <SAW512_NUM_CELLS, MOZZI_AUDIO_RATE> saw5(SAW512_DATA);
Oscil <SAW512_NUM_CELLS, MOZZI_AUDIO_RATE> saw6(SAW512_DATA);
Oscil <SAW512_NUM_CELLS, MOZZI_AUDIO_RATE> saw7(SAW512_DATA);

MultiResonantFilter<uint8_t> mf0;
MultiResonantFilter<uint8_t> mf1;

AudioDelayFeedback <0x800> aDel;



// Global vars

uint16_t chords[] = {
  (0 << 0) + (7 << 4) + (12 << 8) + (12 << 12), // Base note
  (0 << 0) + (0 << 4) + (12 << 8) + (12 << 12),
  (0 << 0) + (0 << 4) + (12 << 8) + (12 << 12),
  (0 << 0) + (0 << 4) + (12 << 8) + (12 << 12),
  (0 << 0) + (0 << 4) + (12 << 8) + (12 << 12),
  (0 << 0) + (0 << 4) + (12 << 8) + (12 << 12),
  (0 << 0) + (0 << 4) + (12 << 8) + (12 << 12),
  (0 << 0) + (0 << 4) + (12 << 8) + (12 << 12),
  (0 << 0) + (0 << 4) + (12 << 8) + (12 << 12)
};


// Sliders
// Real-time position
uint8_t l_position = 0xFF;
uint8_t r_position = 0xFF;

bool l_touched = false;
bool r_touched = false;

bool plus_touched_new =  false;
bool minus_touched_new = false;

bool plus_touched_prev =  false;
bool minus_touched_prev = false;

// Control pads
// Actual true/false of all control pads
uint8_t chord_pad_bin = 0b00000000;
// 0 0 0 N W E S C

uint16_t note_pad_bin = 0b0000000000000000;
// 0 0 0 0 VII VI# VI V# V IV# IV III II# II I# I

// +- pad and ABC
uint8_t set_pad_bin = 0b00000000;
// 0 0 0 + - B C A


// The most recent control pad or direction that has been touched; 0xFF = none touched at the moment
//uint8_t chord_pad = 0xFF;  // this has 5 pads but 9 directions; enumerated 0-8
//uint8_t note_pad =  0xFF;

//uint8_t chord_number = 0x00;
//uint8_t note_pad =  0xFF;

uint16_t chord = chords[0];
uint8_t note;

// Transposition
uint8_t octave_transpose = 2;
uint8_t note_transpose =   0;

int8_t osc_transpose = 24;


// Corresponding values' positions
// default
uint8_t bend_position =    0x80;
uint8_t detune_position =  0x00;

// A set
uint8_t phase_rate =  0x00;
uint8_t phase_depth = 0x00;

// B setczczc
uint8_t filter_position = 0x80;
uint8_t filter_res =      0x00;
// +- pads: filter type
uint8_t filter_type =  LOWPASS;

// C set

// +- pads: note transposition



bool note_chord_spread_change = true;
bool filter_change = true;
bool delay_change = true;




// the delay time, measured in samples, updated in updateControl, and used in updateAudio
Q16n16 del_samps = 0x800;
uint8_t del_fb = 5;

float spread = 0.1;

bool drive = 0;





void updateParameters() {
  switch ((0b00011000 & set_pad_bin) >> 3){ // Update plus minus pads
    case PLUS:
      if (plus_touched_prev) {plus_touched_new = 0;}
      else {plus_touched_new = 1; plus_touched_prev = 1;}
      break;

    case MINUS:
      if (minus_touched_prev) {minus_touched_new = 0;}
      else {minus_touched_new = 1; minus_touched_prev = 1;}
      break;
    
    default:
      minus_touched_prev = 0;
      plus_touched_prev = 0;

      minus_touched_new = 0;
      plus_touched_new = 0;
  }


  switch (0b00000111 & set_pad_bin) { // ABC pads settings
    case A: // Set mode A
      if (l_touched) {phase_rate = l_position;}
      if (r_touched) {phase_depth = r_position;}
      break;

    case B: // Set mode B
      if (l_touched) {filter_position = l_position;}
      if (r_touched) {filter_res = r_position;}

      if (plus_touched_new) {filter_type++;}
      if (minus_touched_new) {filter_type--;}
      if (filter_type >= FILTER_MODES_COUNT) {filter_type = 0;}
      break;

    case C: // Set mode C

      if (plus_touched_new) {note_transpose++;}
      if (minus_touched_new) {note_transpose--;}
      if (note_transpose >= 12) {note_transpose = 0;}
      break;

    default: // No set mode selected; default behavoir
      if (l_touched) {bend_position = l_position;} // Pitchbend
      else {bend_position = 0x80;} // Snap back to center
      if (r_touched) {
        detune_position = r_position;
        spread = (float) detune_position * (octave_transpose + (note_transpose / 12)) / 255;
      }

      if (plus_touched_new) {octave_transpose++;}
      if (minus_touched_new) {octave_transpose--;}
      if (octave_transpose >= MAX_OCTAVE) {octave_transpose = 0;}
  }

  switch (chord_pad_bin) { // Set chord shape to be played
    case 0b00000000:
      break;

    case 0b00000001:
      chord = chords[0];
      break;
  }
}


void setup() {
  // Hardware setup
  // MIDI
  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin(0);
  }

  Serial.begin(115200);
  MIDI.begin(MIDI_CHANNEL_OMNI);

  if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(10);
    TinyUSBDevice.attach();
  }


  usb_midi.setStringDescriptor("TinyUSB MIDI");

  Serial.println("Ok");

  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);

  //Wire.begin();

  // Touch

  //sliderL.begin();
  //sliderR.begin();

  Serial.println("Wire");

  Wire.begin();
  Wire.setClock(1000000);

  // Audio setup

  startMozzi(); // :)

}

void loop(){
  audioHook(); // required here
}

void updateControl(){
  // put changing controls in here

  // MIDI
  MIDI.read();


  // Controls
  //int slider_read_L = sliderL.readSlider();
  //int slider_read_R = sliderR.readSlider();

  //bool l_touched = false;
  //bool r_touched = false;

  //if (slider_read_L != -1) {l_position = slider_read_L; l_touched = true; Serial.println(l_position);}
  //if (slider_read_R != -1) {r_position = slider_read_R; r_touched = true; Serial.println(r_position);}


  //Serial.println("Getting wire data");
  setAddressPointer(0);
  Wire.requestFrom(0x69, 4);
  while (Wire.available()) {
    Serial.print((uint8_t)Wire.read(), HEX);
    //Serial.print(' ');
  }
    //Serial.println("Getting wire data");
  setAddressPointer(0);
  Wire.requestFrom(0x69, 4);
  while (Wire.available()) {
    Serial.print((uint8_t)Wire.read(), HEX);
    //Serial.print(' ');
  }
    //Serial.println("Getting wire data");
  setAddressPointer(0);
  Wire.requestFrom(0x69, 4);
  while (Wire.available()) {
    Serial.print((uint8_t)Wire.read(), HEX);
    //Serial.print(' ');
  }
  Serial.println();

  
  updateParameters();

  // Apply changes

  if (note_chord_spread_change) {
    saw0.setFreq(mtof((chord & 0b0000000000001111) + octave_transpose * 12 + note_transpose + note) - spread * 3);
    saw1.setFreq(mtof(((chord & 0b0000000011110000) >> 4) + octave_transpose * 12 + note_transpose + note) - spread);
    saw2.setFreq(mtof(((chord & 0b0000111100000000) >> 8) + octave_transpose * 12 + note_transpose + note) + spread);
    saw3.setFreq(mtof(((chord & 0b1111000000000000) >> 12) + octave_transpose * 12 + note_transpose + note) + spread * 3);

    saw4.setFreq(mtof((chord & 0b0000000000001111) + octave_transpose * 12 + note_transpose + note + osc_transpose) - spread * 3);
    saw5.setFreq(mtof(((chord & 0b0000000011110000) >> 4) + octave_transpose * 12 + note_transpose + note + osc_transpose) - spread);
    saw6.setFreq(mtof(((chord & 0b0000111100000000) >> 8) + octave_transpose * 12 + note_transpose + note + osc_transpose) + spread);
    saw7.setFreq(mtof(((chord & 0b1111000000000000) >> 12) + octave_transpose * 12 + note_transpose + note + osc_transpose) + spread * 3);

    note_chord_spread_change = false;
  }

  if (filter_change) {
    mf0.setCutoffFreqAndResonance(r_position, 0x40);
    mf1.setCutoffFreqAndResonance(r_position, 0x40);
    //filter_change = false;
  }

  if (delay_change) {
    aDel.setFeedbackLevel(del_fb);
    delay_change = false;
  }
  

}



void setAddressPointer(uint8_t address) {
  Wire.beginTransmission(0x69);    // prepare transmission to slave with address 0x69
  Wire.write(address);            // Write just the address
  Wire.endTransmission();
}



AudioOutput updateAudio(){
  int amix1 = (saw0.next() + saw1.next()) >> !drive;
  int amix2 = (saw2.next() + saw3.next()) >> !drive;
  int amix12 = (amix1 + amix2) >> 1;

  mf0.next(amix12);

  int amix3 = (saw4.next() + saw5.next()) >> !drive;
  int amix4 = (saw6.next() + saw7.next()) >> !drive;
  int amix34 = (amix1 + amix2) >> 1;

  mf1.next(amix34);

  //mf0.next((saw0.next() + saw1.next() + saw2.next() + saw3.next())/4);

  //mf1.next((saw4.next() + saw5.next() + saw6.next() + saw7.next())/4);

  int bank1 = mf0.low();
  int bank2 = mf1.low();

  int filt_sum = (bank1 + bank2) >> 1;

  int del_out = aDel.next(filt_sum, del_samps);

  int mixed = (del_out + filt_sum << 1) >> 1;
  
  return MonoOutput::fromAlmostNBit(10, mixed);
}


void handleNoteOn(byte channel, byte pitch, byte velocity) {
  // Log when a note is pressed.

  note = pitch % 12;
  octave_transpose = (uint8_t) pitch / 12;

  note_chord_spread_change = true;
}

void handleNoteOff(byte channel, byte pitch, byte velocity) {
  // Log when a note is released.
}

