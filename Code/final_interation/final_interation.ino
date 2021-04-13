#include "headers.h"

#ifndef LCDPINS
  #define REGSEL  4
  #define CS    5
#endif

int analog_pin = A3;
#define ARR_SIZE(x)(sizeof(x) / sizeof((x)[0]))

// proper driver for the display
U8G2_ST7565_NHD_C12864_F_4W_SW_SPI u8g2(U8G2_R0, SCK , MOSI, CS, REGSEL, U8X8_PIN_NONE);

int largest(int arr[], int n) {
  int max = 0;
  for (int i = 0; i < n; i++) {
    if (arr[i] > max) 
      max = arr[i]; 
  }
  return max;
}

void init_pins() {
  Serial.begin(115200);
  pinMode(REGSEL, OUTPUT);
  pinMode(CS, OUTPUT);

  setup_pwm_decoder();
  setup_analog_decoder(analog_pin);
  // pinMode(A1, OUTPUT);  // debug_led
  // digitalWrite(A1, LOW);
}

void u8g2_prepare(void) {

  u8g2.setFont(u8g2_font_open_iconic_all_2x_t);    // 18 pixel font type. For value labeling
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);                            // set default black color with white background
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
}

void home_screen() {
  char return_icon[2] = {61, '\0'};
  char option_icon[2] = {82, '\0'};
  u8g2.setFont(u8g2_font_open_iconic_arrow_1x_t);
  u8g2.drawStr(112, 0, return_icon);

  u8g2.drawStr(0, 16, option_icon); // or use N
  u8g2.drawStr(0, 32, option_icon);
  u8g2.drawStr(0, 48, option_icon);

  u8g2.setFont(u8g2_font_freedoomr10_mu);
  u8g2.drawStr(16, 16, "DIFF. ENCODER");
  u8g2.drawStr(16, 32, "ANALOG ENCODER");
  u8g2.drawStr(16, 48, "PWM ENCODER");
}

void display_all_screen(int dual_line_value, int single_line_value, int analog_value, int pwm_value) {
  u8g2.setFont(u8g2_font_freedoomr10_mu);
  char labels[4][11] = {"DUAL", "SINGLE", "ANALOG", "PWM"};
  int string_len[4] = {strlen(labels[0]), strlen(labels[1]), strlen(labels[2]), strlen(labels[3])};
  int indent_size = largest(string_len, 4)*10;

  // assert the labels of each encoder
  u8g2.drawStr(0, 0, labels[0]);
  u8g2.drawStr((indent_size), 0, labels[1]);
  u8g2.drawStr(0, 32, labels[2]);
  u8g2.drawStr((indent_size), 32, labels[3]);

  // printing the values from the encoders
  char value_buffer[5] = {'\0'};                                                  // 5 digits is all needed for 16 bit
  int args[4] = {dual_line_value, single_line_value, analog_value, pwm_value};    // re-assigning for more condensed code
  
  for (int i = 0; i < 4; i++) {
    if(args[i] == -1) {
      snprintf(value_buffer, 5, "%s", "(/)");                                     // disconnected encoder representation
    } else {
      snprintf(value_buffer, 5, "%d", args[i]);                                   // convert to string to be printed
    }
    
    if(i == 0) {
      u8g2.drawStr(0, 16, value_buffer);
    }
    else if(i == 1) {
      u8g2.drawStr(indent_size, 16, value_buffer);
    }
    else if(i == 2) {
      u8g2.drawStr(0, 48, value_buffer);
    }
    else if(i == 3) {
      u8g2.drawStr(indent_size, 48, value_buffer);
    } 
    memset(value_buffer, '\0', sizeof(value_buffer));
  }
}

void setup() {
  init_pins();
  ls7366r_init(CS1);
  ls7366r_init(CS2);
//  u8g2.begin();   // calls init, clears, and disable power saving mode.

//  u8g2.firstPage();
//  u8g2_prepare();
//  home_screen();
//  u8g2.sendBuffer();

  // Allow some time todemo home screen
  delay(3000);    
}

void loop() {
//  u8g2.firstPage();
//  u8g2_prepare();
int value = map(analogRead(analog_pin), 0, 670, 0, 100);
int value2 = ls7366r_read_cntr(CS1);
int value3 = ls7366r_read_cntr(CS2);
int value4 = pwm_reading;

     Serial.print(value); Serial.print(' ');
     Serial.print(value2); Serial.print(' ');
     Serial.print(value3); Serial.print(' ');
     Serial.println(value4);
  //delay(50);

//  display_all_screen(ls7366r_read_cntr(CS1), ls7366r_read_cntr(CS1), analogRead(analog_pin), pwm_reading);
  u8g2.sendBuffer();
}
