# Frequency-aware Big Mouth Billy Bass

This project brings new life to the iconic Big Mouth Billy Bass by transforming it into fully functional Bluetooth speaker with simple animatronics.
Billy Bass can move its head, tail, and mouth in response to different audio frequencies in music.

There are numerous Billy Bass projects, but I haven't found any that provided frequency-aware code. So I created one. And it works pretty good!

[Video](https://youtu.be/alQjzy3bQl0)

## Features
- **Mouth movement**: Responds to vocal frequencies.
- **Head and tail movement**: Controlled by bass/rhythmic frequencies.
- **Bluetooth audio input**: Enables wireless streaming from your phone or other devices.

## Components
- **Arduino Uno Rev 3**
- **Arduino Motor Shield Rev 3**
- **Audio stereo amplifier PAM8403**
- **Bluetooth audio stereo module A2DP VHM-314**
- **2W/8Ohm speaker**
- **Big Mouth Billy Bass (modified for servo and motor control)**

## Requirements
Ensure that you have the following library installed in your Arduino IDE:
- `arduinoFFT` library: This library is used to perform Fast Fourier Transform, which breaks down audio signals into frequency bands. Install it via the Library Manager or download it from [arduinoFFT GitHub](https://github.com/kosme/arduinoFFT).

## How it works?
The audio signal received from the Bluetooth module is amplified by the PAM8403 amplifier and fed into speaker. Additional wiring from Bluetooth module is connected to the Arduino and measured in regular intervals. The `arduinoFFT` library processes this audio signal to extract frequency data. Depending on the frequency range:
- **Low frequencies - approximately 100Hz**: Trigger head and tail movements.
- **Higher frequencies - approximately 400Hz - 800Hz**: Activate the mouth movement.
 
