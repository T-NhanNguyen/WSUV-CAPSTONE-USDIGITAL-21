// pwm encoder signal line is connected to digital pin 3

#include "headers.h"

static volatile unsigned long ton = 0;   // set initial times on to 0
static volatile unsigned long toff = 0;  // set inital off time to 0 
static volatile unsigned long time_stamp;  // time stamp
static volatile int dty_ccl = 0;     // set initial duty cycle to 0
static volatile int reading = 0;

volatile int pwm_reading;       // requires a global variable as this is passed between isr and main

// no parameter because this requires a global variable to be passed between isr
void setup_pwm_decoder() {
  pinMode(pwm_signal, INPUT);  //initialize digital 3 pin to input
  attachInterrupt(digitalPinToInterrupt(pwm_signal), rising_edge, RISING);  //set initial interrupt to input pin for rising edge trigger
}

void setup_analog_decoder(int pin) {
  pinMode(pin, INPUT);
}

void rising_edge() {
  digitalRead(pwm_signal);
  time_stamp = micros();  // get current time stamp
  // note: The first calculation of duty cycle will be in error because two ton readings are needed.  After that, all is well.
  dty_ccl = time_stamp - ton;  // Duty Cycle = current rising edge time - previous rising edge time
  ton = time_stamp;        // reads the time of the rising edge
  attachInterrupt(digitalPinToInterrupt(pwm_signal), falling_edge, FALLING);
}

void falling_edge() {
  toff = micros();      // reads time when falling edge is triggered
  pwm_reading = (toff - ton);
  if (pwm_reading > 1024)       // clip in case too large
    pwm_reading = 1024;
  attachInterrupt(digitalPinToInterrupt(pwm_signal), rising_edge, RISING);
}