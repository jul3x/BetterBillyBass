// Created by jul3x on 06.11.2024
// BillyBass animatronic fish reacting to music controlled via Bluetooth 

#include "arduinoFFT.h"

//////// CONFIGURATION ////////////
// FFT constants
const uint16_t audio_samples = 64;       // This value MUST ALWAYS be a power of 2
const float audio_sampling_freq = 9000;  // [Hz] must be less than 10000 due to ADC
unsigned int sampling_period_us;

// PIN connectors contats
const int DIRECTION_PIN_MOUTH = 12;
const int PWM_PIN_MOUTH = 3;
const int DIRECTION_PIN_BODY = 13;
const int PWM_PIN_BODY = 11;
const int SOUND_PIN = A3;

// Configuration for movement/talking
const int OPEN_MOUTH_TIME = 200;   // Opening mouth duration
const int CLOSE_MOUTH_TIME = 100;  // Closing mouth duration
const int MAX_MOUTH_TIME = 1000;   // Max duration of open mouth

const int FORWARD_BODY_TIME = 1000;  // Time to raise body
const int BACKWARD_BODY_TIME = 200;  // Time of returning to standby position
const int MAX_BODY_TIME = 3000;      // Max duration of body movement

// Configuration for audio-movement reactivity
const int VOCAL_FREQ_THRESHOLD = 800;       // Start talking above this level
const int VOCAL_FREQ_MAX_THRESHOLD = 3500;  // Continue talking above this level

const int BASS_FREQ_THRESHOLD = 1600;      // Move body above this level
const int BASS_FREQ_MAX_THRESHOLD = 2500;  // Keep moving above this level
//////// END CONFIGURATION ////////////

// FFT
float v_real[audio_samples];
float v_imag[audio_samples];

ArduinoFFT<float> FFT = ArduinoFFT<float>(v_real, v_imag, audio_samples, audio_sampling_freq, true);

// Current readings
unsigned long current_time;
unsigned long microseconds;
int current_sound_volume;
unsigned long bass;
unsigned long vocal;

// Talking state
int talking_phase = 0;
unsigned long talking_phase_switch_ts = 0;
unsigned long max_time_mouth_ts = 0;

// Moving state
int body_phase = 0;
unsigned long body_phase_switch_ts = 0;
unsigned long max_time_body_ts = 0;

void setup() {
  pinMode(DIRECTION_PIN_BODY, OUTPUT);
  pinMode(PWM_PIN_BODY, OUTPUT);
  pinMode(DIRECTION_PIN_MOUTH, OUTPUT);
  pinMode(PWM_PIN_MOUTH, OUTPUT);
  pinMode(SOUND_PIN, INPUT);

  analogWrite(PWM_PIN_BODY, 0);
  analogWrite(PWM_PIN_MOUTH, 0);

  sampling_period_us = round(1000000 * (1.0 / audio_sampling_freq));
}

void loop() {
  current_time = millis();
  sampleMusic();
  BillyBass();
}

void sampleMusic() {
  // Perform FFT and calculate bass / vocal values

  microseconds = micros();
  for (int i = 0; i < audio_samples; i++) {
    v_real[i] = analogRead(SOUND_PIN);
    v_imag[i] = 0;
    while (micros() - microseconds < sampling_period_us) {}
    microseconds += sampling_period_us;
  }

  FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);
  FFT.compute(FFTDirection::Forward);
  FFT.complexToMagnitude();

  // Calculate bass
  bass = 0;
  for (int i = 1; i < 2; i++) {
    bass += v_real[i];  // approx 148Hz
  }

  // Calculate vocal
  vocal = 0;
  for (int i = 2; i < 10; i++) {
    vocal += v_real[i];  // approx 240Hz - 800 Hz
  }
}

void BillyBass() {
  // BillyBass movement logic

  // Head movement
  talkLoop(OPEN_MOUTH_TIME, CLOSE_MOUTH_TIME);
  if (vocal >= VOCAL_FREQ_THRESHOLD && talking_phase == 0) {
    // If not talking and vocal frequency is above threshold,
    // start talking and trigger state machine.
    talking_phase = 1;
    talking_phase_switch_ts = current_time;
    max_time_mouth_ts = current_time + MAX_MOUTH_TIME;
  } else if (vocal >= VOCAL_FREQ_MAX_THRESHOLD && current_time <= max_time_mouth_ts) {
    // If Billy is currently talking but vocal frequencies are very loud (above second threshold),
    // Prolongate talking.
    talking_phase_switch_ts = current_time + OPEN_MOUTH_TIME - 10;
  }

  // Body movement
  moveLoop(FORWARD_BODY_TIME, BACKWARD_BODY_TIME, FORWARD_BODY_TIME, BACKWARD_BODY_TIME);
  if (bass >= BASS_FREQ_THRESHOLD && body_phase == 0) {
    // If Billy is in standby but bass frequencies are above threshold,
    // Start moving and trigger state machine.

    // Sometimes tail movement, sometimes head
    bool tail = random(1, 10) > 6;
    body_phase = tail ? 3 : 1;

    body_phase_switch_ts = current_time;
    max_time_body_ts = current_time + MAX_BODY_TIME;
  } else if (bass >= BASS_FREQ_MAX_THRESHOLD && current_time <= max_time_body_ts) {
    // If Billy is currently moving but bass frequencies are very loud (above second threshold),
    // Prolongate movement.
    body_phase_switch_ts = current_time + FORWARD_BODY_TIME - 10;
  }
}

void talkLoop(int openTime, int closeTime) {
  // Talking state machine

  if (talking_phase == 0) {
    // Standby
  } else if (talking_phase == 1) {
    openMouth();

    if (talking_phase_switch_ts + openTime <= current_time) {
      talking_phase = 2;
      talking_phase_switch_ts = current_time;
    }
  } else if (talking_phase == 2) {
    closeMouth();

    if (talking_phase_switch_ts + closeTime <= current_time) {
      talking_phase = 0;
      talking_phase_switch_ts = current_time;
    }
  }
}

void moveLoop(int headUpTime, int headDownTime, int tailUpTime, int tailDownTime) {
  // Body movement state machine

  if (body_phase == 0) {
    // Standby
  } else if (body_phase == 1) {
    flapHead();

    if (body_phase_switch_ts + headUpTime <= current_time) {
      body_phase = 2;
      body_phase_switch_ts = current_time;
    }
  } else if (body_phase == 2) {
    stopBody();

    if (body_phase_switch_ts + headDownTime <= current_time) {
      body_phase = 0;
      body_phase_switch_ts = current_time;
    }
  } else if (body_phase == 3) {
    flapTail();

    if (body_phase_switch_ts + tailUpTime <= current_time) {
      body_phase = 4;
      body_phase_switch_ts = current_time;
    }
  } else if (body_phase == 4) {
    stopBody();

    if (body_phase_switch_ts + tailDownTime <= current_time) {
      body_phase = 0;
      body_phase_switch_ts = current_time;
    }
  }
}

void openMouth() {
  digitalWrite(DIRECTION_PIN_MOUTH, HIGH);
  analogWrite(PWM_PIN_MOUTH, 255);
}

void closeMouth() {
  digitalWrite(DIRECTION_PIN_MOUTH, HIGH);
  analogWrite(PWM_PIN_MOUTH, 0);
}

void flapTail() {
  analogWrite(PWM_PIN_BODY, 255);
  digitalWrite(DIRECTION_PIN_BODY, LOW);
}

void flapHead() {
  analogWrite(PWM_PIN_BODY, 255);
  digitalWrite(DIRECTION_PIN_BODY, HIGH);
}

void stopBody() {
  analogWrite(PWM_PIN_BODY, 0);
}
