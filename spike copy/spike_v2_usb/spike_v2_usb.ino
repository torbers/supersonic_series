
#include "Spike.h"

#include <Mozzi.h>
#include <Oscil.h>
#include <tables/saw512_int8.h>
#include <ResonantFilter.h>
#include <mozzi_midi.h> // for mtof
#include <AudioDelayFeedback.h>
#include <ADSR.h>

#include <USB-MIDI.h>

#include <Adafruit_NeoPixel.h>

#define MOZZI_CONTROL_RATE 256
//#define MOZZI_AUDIO_RATE 16384

Spike spike;
Adafruit_NeoPixel pixel(1, 8, NEO_GRB + NEO_KHZ800);

//USBMIDI_Interface midi;

//Adafruit_USBD_MIDI usb_midi;
//MIDI_CREATE_INSTANCE(USBMIDI_Interface, midi, MIDI);
USBMIDI_CREATE_DEFAULT_INSTANCE();

bool new_midi = false;


uint8_t note = 0;
uint8_t base = 36;
int8_t oplus = 12;



uint8_t chords[9][4] = {
  {0, 12, 0, 12}, // Center
  {0, 4, 7, 12},  // n maj
  {0, 5, 7, 12},  // nw sus4
  {0, 3, 7, 12},  // w min
  {0, 3, 7, 10},  // we min7
  {0, 4, 7, 11},  // s 
  {0, 2, 7, 12},  // se sus2
  {0, 7, 7, 12},  // e 5th
  {0, 4, 7, 11}   // ne maj7
};



// Audio objects
// Oscillators
Oscil <SAW512_NUM_CELLS, MOZZI_AUDIO_RATE> saw0(SAW512_DATA);
Oscil <SAW512_NUM_CELLS, MOZZI_AUDIO_RATE> saw1(SAW512_DATA);
Oscil <SAW512_NUM_CELLS, MOZZI_AUDIO_RATE> saw2(SAW512_DATA);
Oscil <SAW512_NUM_CELLS, MOZZI_AUDIO_RATE> saw3(SAW512_DATA);
Oscil <SAW512_NUM_CELLS, MOZZI_AUDIO_RATE> saw4(SAW512_DATA);
Oscil <SAW512_NUM_CELLS, MOZZI_AUDIO_RATE> saw5(SAW512_DATA);
Oscil <SAW512_NUM_CELLS, MOZZI_AUDIO_RATE> saw6(SAW512_DATA);
Oscil <SAW512_NUM_CELLS, MOZZI_AUDIO_RATE> saw7(SAW512_DATA);
float spread = 0.001;

// Filter
MultiResonantFilter<uint8_t> mf0;
MultiResonantFilter<uint8_t> mf1;
uint8_t filter_position = 0xFF;
uint8_t filter_res =      0x80;
int8_t filter_type =  HIGHPASS;


// Delay effect
AudioDelayFeedback <0x0800> aDel;
Q16n16 del_samps = 0x00000001;
uint8_t del_fb = 0x00;

// Amp effects
bool drive = 0;

// envelope
ADSR <MOZZI_CONTROL_RATE, MOZZI_CONTROL_RATE> envelope;
unsigned int attack = 0;
unsigned int decay = 0;
unsigned int sustain = 1000000;
unsigned int release_ms = 255;
bool do_envelope = true;
uint8_t gain = 0;


uint8_t mode = 0;

uint8_t chord_index = 0;


void setMode() {
  if      ((!spike.pad_a) && (!spike.pad_b) && (!spike.pad_c)) {mode = 0; pixel.setPixelColor(0, pixel.Color(  0,   0, 255));}
  else if (( spike.pad_a) && (!spike.pad_b) && (!spike.pad_c)) {mode = 1; pixel.setPixelColor(0, pixel.Color(200,   0,   0));} // A
  else if (( spike.pad_a) && ( spike.pad_b) && (!spike.pad_c)) {mode = 2; pixel.setPixelColor(0, pixel.Color(150, 150,   0));}
  else if ((!spike.pad_a) && ( spike.pad_b) && (!spike.pad_c)) {mode = 3; pixel.setPixelColor(0, pixel.Color(  0, 200,   0));} // B
  else if ((!spike.pad_a) && ( spike.pad_b) && ( spike.pad_c)) {mode = 4; pixel.setPixelColor(0, pixel.Color(  0, 120, 250));}
  else if ((!spike.pad_a) && (!spike.pad_b) && ( spike.pad_c)) {mode = 5; pixel.setPixelColor(0, pixel.Color(  50,   50, 250));} // C
  else if (( spike.pad_a) && (!spike.pad_b) && ( spike.pad_c)) {mode = 6; pixel.setPixelColor(0, pixel.Color(150,   0, 250));}

  pixel.show();
}

void setChord() {
  if      ((!spike.pad_n) && (!spike.pad_w) && (!spike.pad_s) && (!spike.pad_e)) {chord_index = 0;} // none

  else if (( spike.pad_n) && (!spike.pad_w) && (!spike.pad_s) && (!spike.pad_e)) {chord_index = 1;} // n maj
  else if (( spike.pad_n) && ( spike.pad_w) && (!spike.pad_s) && (!spike.pad_e)) {chord_index = 2;} // nw sus4
  else if ((!spike.pad_n) && ( spike.pad_w) && (!spike.pad_s) && (!spike.pad_e)) {chord_index = 3;} // w min
  else if ((!spike.pad_n) && ( spike.pad_w) && (!spike.pad_s) && ( spike.pad_e)) {chord_index = 4;} // we min7
  else if ((!spike.pad_n) && (!spike.pad_w) && ( spike.pad_s) && (!spike.pad_e)) {chord_index = 5;} // s dim
  else if ((!spike.pad_n) && (!spike.pad_w) && ( spike.pad_s) && ( spike.pad_e)) {chord_index = 6;} // se sus2
  else if ((!spike.pad_n) && (!spike.pad_w) && (!spike.pad_s) && ( spike.pad_e)) {chord_index = 7;} // e 5th
  else if (( spike.pad_n) && (!spike.pad_w) && (!spike.pad_s) && ( spike.pad_e)) {chord_index = 8;} // ne maj7

}


void doSliderChanges(){
  if (spike.slider_touched_l) {
    pixel.setPixelColor(0, pixel.Color(spike.slider_pos_l / 2 + 128, 0, 255));
  }
  else if (spike.slider_touched_r) {
    pixel.setPixelColor(0, pixel.Color(0, spike.slider_pos_r / 2 + 128, 255));
  }

  if (mode == 0) {
    if (spike.slider_touched_l) {
      filter_position = spike.slider_pos_l;
      mf0.setCutoffFreqAndResonance(filter_position, filter_res);
      mf1.setCutoffFreqAndResonance(filter_position, filter_res);
    }
    else if (spike.slider_touched_r) {
      filter_res = spike.slider_pos_r;
      mf0.setCutoffFreqAndResonance(filter_position, filter_res);
      mf1.setCutoffFreqAndResonance(filter_position, filter_res);
    }
  }

  else if (mode == 1) {
    if (spike.slider_touched_l) {
      del_samps = ((spike.slider_pos_l * 8) << 16) + 1;
    }
    else if (spike.slider_touched_r) {
      del_fb = spike.slider_pos_r;
      aDel.setFeedbackLevel((127 - del_fb / 2) + 127);
    }
  }

  else if (mode == 3) {
    if (spike.slider_touched_l) {
      attack = spike.slider_pos_l * 2;
      envelope.setTimes(attack,decay,sustain,release_ms);
      
    }
    else if (spike.slider_touched_r) {
      do_envelope = (spike.slider_pos_r <= 240);
      release_ms = spike.slider_pos_r * 2;
      envelope.setTimes(attack,decay,sustain,release_ms);
    }
    
  }

  else if (mode == 5) {
    if (spike.slider_touched_l) {
      if (spike.slider_pos_l < 128) {spread = spike.slider_pos_l / 32768.0;}
      else {spread = (spike.slider_pos_l - 128) / 2048.0 + 0.00390625;}
    }
    else if (spike.slider_touched_r) {
      oplus = (((int) spike.slider_pos_r / 32) - 2) * 12;
    }
  }

  pixel.show();
}


void doPMChanges(){
  int8_t pm = spike.getPM();
  if (mode == 0) {
    if (base > 12 && pm == -1){
      base -= 12;
    }
    else if (base < 243 && pm == 1) {
      base += 12;
    }
  }
  else if (mode == 1) {filter_type += pm; if (filter_type < LOWPASS) {filter_type = HIGHPASS;} else if (filter_type > HIGHPASS) filter_type = LOWPASS;}
  else if (mode == 5) {base += pm;}
}


void setup() {

  delay(100);
  
  Serial.begin(115200);

  MIDI.begin(MIDI_CHANNEL_OMNI);


  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);

  MIDI.turnThruOff();


  spike.begin();
  pixel.begin();
  startMozzi();
  envelope.setLevels(255,255,255,0);
  envelope.setTimes(attack,decay,sustain,release_ms);


}

void loop() {
  audioHook();
}

void updateControl(){
  spike.update();
  if (spike.note_touched) {note = spike.getNote();}

  MIDI.read();

  setMode();
  setChord();
  doSliderChanges();
  doPMChanges();

  //Serial.println(spike.mpr0_touched, 2);// Serial.println(spike.mpr1_touched, 2);

  
  //spike.getChord();
  //uint8_t chord_index = spike.chord_index;


  if (true) {//(spike.mpr1_touched || new_midi || spike.mpr0_touched) {
    //if (new_midi) {new_midi = false;}
    saw0.setFreq(mtof(base + note + chords[chord_index][0]) * (1 + spread * 3));
    saw1.setFreq(mtof(base + note + chords[chord_index][1]) * (1 - spread));
    saw2.setFreq(mtof(base + note + chords[chord_index][2]) * (1 + spread));
    saw3.setFreq(mtof(base + note + chords[chord_index][3]) * (1 - spread * 3));
    saw4.setFreq(mtof(base + note + oplus + chords[chord_index][0]) * (1 + spread * 3));
    saw5.setFreq(mtof(base + note + oplus + chords[chord_index][1]) * (1 - spread));
    saw6.setFreq(mtof(base + note + oplus + chords[chord_index][2]) * (1 + spread));
    saw7.setFreq(mtof(base + note + oplus + chords[chord_index][3]) * (1 - spread * 3));
  }

  if (spike.note_touched) {envelope.noteOn(true);}
  if (spike.all_off_now) {envelope.noteOff();}


  envelope.update();
  gain = envelope.next();
  //Serial.println(oplus);
}

AudioOutput updateAudio(){
  int amix1 = (saw0.next() + saw1.next()) >> !drive;
  int amix2 = (saw2.next() + saw3.next()) >> !drive;
  int amix12 = (amix1 + amix2) >> 1;

  mf0.next(amix12);

  int amix3 = (saw4.next() + saw5.next()) >> !drive;
  int amix4 = (saw6.next() + saw7.next()) >> !drive;
  int amix34 = (amix3 + amix4) >> 1;

  if (oplus < 0) {amix34 = 0;}

  mf1.next(amix34);

  //mf0.next((saw0.next() + saw1.next() + saw2.next() + saw3.next())/4);

  //mf1.next((saw4.next() + saw5.next() + saw6.next() + saw7.next())/4);
  int bank1;
  int bank2;
  
  if      (filter_type == LOWPASS ) {bank1 =  mf0.low(); bank2 =  mf1.low();}
  else if (filter_type == BANDPASS) {bank1 = mf0.band(); bank2 = mf1.band();}
  else if (filter_type == HIGHPASS) {bank1 = mf0.high(); bank2 = mf1.high();}

  int filt_sum = (bank1 + bank2) >> 1;

  if (do_envelope) {filt_sum = ((filt_sum * gain) / 256);}

  int del_out = aDel.next(filt_sum, del_samps);

  int mixed = (del_out + filt_sum << 1) >> 1;
  return MonoOutput::fromAlmostNBit(9, (mixed));
}





void handleNoteOn(byte channel, byte pitch, byte velocity) {
  // Log when a note is pressed.
  note = pitch % 12;
  envelope.noteOn(true);
  new_midi = true;
  base = ((uint8_t) pitch / 12) * 12;
}

void handleNoteOff(byte channel, byte pitch, byte velocity) {
  // Log when a note is released.
  envelope.noteOff();
}
