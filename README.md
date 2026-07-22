# Supersonic Series

Airplane dragon synthesizers

## Spike chord synthesizer
### Instruction card
<img width="695" height="695" alt="image" src="https://github.com/user-attachments/assets/76c7dc62-90eb-4a18-a9da-f723dcc065f2" />

### MIDI CC implementation
| Controller | Effect |
| --- | --- |
| 70 | Chord shape |
| 71 | Filter resonance |
| 72 | Amp envelope attack |
| 73 | Amp envelope release |
| 74 | Filter frequency |
| 75 | Delay time |
| 76 | Delay feedback |
| 77 | Detune |
| 78 | Oscillator bank spread |

### Building Arduino code
The Arduino IDE is the easiest, most portable way to compile and upload code to Spike. This section assumes no previous knowledge of the Arduino IDE in order to accommodate coders of all experience levels.
#### 1. Install Arduino IDE
Follow [this tutorial](https://docs.arduino.cc/software/ide-v2/tutorials/getting-started/ide-v2-downloading-and-installing/) to install the Arduino IDE 2 on your computer. The environment supports Windows, Mac, and Linux.

#### 2. Clone this repository
Use your preferred method to "clone" (download) this repository to your computer. I like the [github desktop app](https://desktop.github.com/download/), but you can also just download the whole thing as a .zip from the [main page](https://github.com/torbers/supersonic_series).

#### 3. Open the file
Once the IDE has installed, choose `File->Open` and navigate to and open `supersonic_series/spike copy/spike_v2_usb/spike_v2_usb.ino`. **Make sure** `spike_v2_usb.ino` and the other files stay in the `spike_v2_usb` folder or it won't open correctly!

#### 4. Install libraries
You will need to install 3 libraries from the built-in library manager (the book icon on the side of the Arduino window).
- **Mozzi** by Tim Barrass: This is our sound synthesis library
- **USB-MIDI** by lathoub: This library sends and recieves MIDI over USB
- **Adafruit_NeoPixel** by Adafruit: This library controls the built in LED

#### 5. Connect Spike
Connect Spike to your computer via the USB C port. **Make sure your cable is capable of data transmission or "sync," and isn't a power-only cable.**
It will appear in Arduino as `Arduino Zero (Native USB)` at the top of the screen. Make sure the text saying this is bold - if so, you're ready to upload.

#### 6. Compile and upload!
Press the arrow button at the top left of the screen to compile and upload the code.

---
## Licensing
All work is shared under [Creative Commons CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/): Anyone is free to share, remix, and commercialize it so long as they attribute the work and release it under this same license agreement. (Attribution text modified from [here](https://fullyautomatedrpg.com/licensing/).)
