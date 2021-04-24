#include <SPI.h>
#include <Arduino.h>
#include <U8g2lib.h>

#ifndef LCDPINS
  #define REGSEL  4
  #define CS    5
#endif

#ifndef DECODE_CS
  #define CS1 6
  #define CS2 12
#endif

#ifndef PWM_PIN
  #define pwm_signal 2             //digital pin 11 did not work
#endif

U8G2_ST7565_NHD_C12864_F_4W_SW_SPI u8g2(U8G2_R0, SCK , MOSI, CS, REGSEL, U8X8_PIN_NONE);

static const int analog_pin = A3;
extern volatile long pwm_reading;

// display
void display_init();
void u8g2_prepare(void);
void display_all_screen(int dual_line_value, int single_line_value, int analog_value, int pwm_value);
int largest(int arr[], int n);

// quadrature encoder
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

void goto_to_homescreen() {
  u8g2.firstPage();
  u8g2_prepare();
  home_screen();
  u8g2.sendBuffer();
  while(1){
    if(!digitalRead(10) || !digitalRead(9)) {
      delay(500);
      break;
    }
  }
}

void setup() {
  Serial.begin(9600);
  // NOTE: LS7366R opperates faster than arduino
  SPI.begin();

  pinMode(10, INPUT);
  pinMode(9, INPUT);
  
  setup_pwm_decoder();
  setup_analog_decoder(analog_pin);
  
  ls7366r_init(CS1);
  ls7366r_init(CS2);

  SPI.end();        // this interfere with the display's api somehow?...
  display_init();
  
  goto_to_homescreen();
}

void loop() {
  SPI.begin();
  int analog_encoder = map(analogRead(analog_pin), 0, 670, 0, 100);
  int sing_line = ls7366r_read_cntr(CS1);
  int dual_line = ls7366r_read_cntr(CS2);
  int pwm_encoder = map(pwm_reading, 0, 1024, 0, 100);;
  SPI.end();

  u8g2.firstPage();
  u8g2_prepare();
  // display_all_screen(-1, -1, -1, -1);
  display_all_screen(dual_line, sing_line, analog_encoder, pwm_encoder);
  u8g2.sendBuffer();

  if(!digitalRead(10))
    goto_to_homescreen();
}