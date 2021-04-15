#include <SPI.h>
#include <U8g2lib.h>
#include <util/delay.h>            // Delay functions
#include <SoftwareSerial.h>        // Serial Library
#include "Arduino.h"               // Arduino
#include "decoder_macros.h"

#ifndef DECODE_CS
  #define CS1 6
  #define CS2 12
#endif
#ifndef PWM_PIN
  #define pwm_signal 2             //digital pin 11 did not work
#endif
extern volatile int pwm_reading;

// quadrature decoder
long int ls7366r_read_cntr(uint8_t device);
uint8_t ls7366r_recieve(uint8_t device, uint8_t op_code);
void ls7366r_write(uint8_t device, uint8_t op_code, uint8_t data);
void ls7366r_init(uint8_t device);
long int ls7366r_read_cntr(uint8_t device);
void print_concatstr(char *sentence, int value);

// pwm decoder
void setup_pwm_decoder();
void rising_edge();
void falling_edge();

// analog decoder
void setup_analog_decoder(int pin);


// display
int largest(int arr[], int n);
void u8g2_prepare(void);
void home_screen();
void display_all_screen(int dual_line_value, int single_line_value, int analog_value, int pwm_value);
void test_screen(void);