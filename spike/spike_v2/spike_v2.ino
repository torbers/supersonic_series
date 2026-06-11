
#include "Spike.h"

#include <Mozzi.h>
#include <Oscil.h>
#include <tables/saw512_int8.h>
#include <ResonantFilter.h>
#include <mozzi_midi.h> // for mtof
#include <AudioDelayFeedback.h>

#define MOZZI_CONTROL_RATE 256
#define MOZZI_AUDIO_RATE 16384

Spike spike;

uint8_t note;
uint8_t base = 36;
uint8_t oplus = 12;



uint8_t chords[9][4] = {
  {0, 12, 0, 12}, // Center
  {0, 4, 7, 12},  // N   Major
  {0, 0, 0, 12},  // NW  
  {0, 2, 7, 12},  // W   Sus2
  {0, 0, 0, 12},  // SW
  {0, 3, 7, 12},  // S   Minor
  {0, 0, 0, 12},  // SE
  {0, 6, 7, 12},  // E   Sus4
  {0, 0, 0, 12}   // NE
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
uint8_t filter_position = 0x80;
uint8_t filter_res =      0x00;
uint8_t filter_type =  LOWPASS;


// Delay effect
AudioDelayFeedback <0x800> aDel;
Q16n16 del_samps = 0x000;
uint8_t del_fb = 0x00;

// Amp effects
bool drive = 0;



void setup() {
  Serial.begin(115200);
  spike.begin();
  startMozzi();
}

void loop() {
  audioHook();
}

void updateControl(){
  spike.update();
  Serial.println(spike.u1_read[0]);
  note = spike.getNote();
  spike.getChord();

  mf0.setCutoffFreqAndResonance(filter_position, filter_res);
  mf1.setCutoffFreqAndResonance(filter_position, filter_res);

  uint8_t chord_index = spike.chord_index;

  if (note < 0xFF) {
    saw0.setFreq(mtof(base + note + chords[chord_index][0]) * (1 + spread * 3));
    saw1.setFreq(mtof(base + note + chords[chord_index][1]) * (1 - spread));
    saw2.setFreq(mtof(base + note + chords[chord_index][2]) * (1 + spread));
    saw3.setFreq(mtof(base + note + chords[chord_index][3]) * (1 - spread * 3));
    saw4.setFreq(mtof(base + note + oplus + chords[chord_index][0]) * (1 + spread * 3));
    saw5.setFreq(mtof(base + note + oplus + chords[chord_index][1]) * (1 - spread));
    saw6.setFreq(mtof(base + note + oplus + chords[chord_index][2]) * (1 + spread));
    saw7.setFreq(mtof(base + note + oplus + chords[chord_index][3]) * (1 - spread * 3));
  }
  else {
    mf0.setCutoffFreqAndResonance(0, filter_res);
    mf1.setCutoffFreqAndResonance(0, filter_res);
  }

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
  
  return MonoOutput::fromAlmostNBit(9, mixed);
}
