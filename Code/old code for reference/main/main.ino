#include <SPI.h>
#include <Wire.h>
#include <Arduino.h>
#include <U8g2lib.h>
#ifndef LCDPINS
  #define REGSEL  4
  #define CS    5
#endif
#define ARR_SIZE(x)(sizeof(x) / sizeof((x)[0]))

U8G2_ST7565_NHD_C12864_F_4W_SW_SPI u8g2(U8G2_R0, SCK , MOSI, CS, REGSEL, U8X8_PIN_NONE);

int logic_out[3] = {A1, A2, A3};          // A1 = Index pin. A2 = A channel. A3 = B Channel
int sing_logic_out[3] = {10, 11};           // D7 = A, D8 = B, -99 is placeholder
int active_state[2] = {A4, A5};             // D5 = Active-high select, D6 = Active-low select

// theres an accidental short between ltc485_ro and one button
// the leds are soldered at the wrong orientation
// Q1 for power led indicatior footprint is incorrect
// ADC pins are not being used to read analog pin.
const int ltc485_ro = 0;
const int ltc485_roe = A5;
const int ltc485_di = 1;
const int button_pin[4] = {10, 1, 9, 0};
int pwm_input = 3;                    //set pwm_input pin to digital pin 3
const int analog_logic = 7;
volatile unsigned long ton = 0;       // set initial times on to 0
volatile unsigned long toff = 0;      // set inital off time to 0 
volatile unsigned long time_stamp;    // time stamp
volatile int x = 0;                   // set initial position x reading as 0
volatile int dty_ccl = 0;             // set initial duty cycle to 0
volatile int reading = 0;


void init_pins();
void analog_ecoder(int *counter);
void dl_encoder(unsigned char *old_state, unsigned char *new_state, int channels[], int array_size, int *counter);
void testdrawline();
void u8g2_prepare(void);
void home_screen();
void display_all_screen(int dual_line_value, int single_line_value, int analog_value, int pwm_value);
void rising_edge();
void falling_edge();

int largest(int arr[], int n) {
  int max = 0;
  for (int i = 0; i < n; i++) {
    if (arr[i] > max) 
      max = arr[i]; 
  }
  return max;
}
// essentially the main
void setup() {
  init_pins();
  u8g2.begin();   // calls init, clears, and disable power saving mode.

  int counter = 0;
  unsigned char old_state, new_state;
  uint32_t button_state[4] = {0};

  u8g2.firstPage();
  u8g2_prepare();
  home_screen();
  u8g2.sendBuffer();
  delay(3000);    // demo home screen
  old_state = new_state = ((digitalRead(logic_out[1]) << 1) | digitalRead(logic_out[2]));

}

void loop() {
  u8g2.firstPage();
  u8g2_prepare();

  delay(500);

  display_all_screen(-1, -1, -1, -1);
  u8g2.sendBuffer();
}

void init_pins() {
  Serial.begin(115200);
  for(int i = 0; i < 3; i++){
    if(i < 2) {
      pinMode(logic_out[i], INPUT_PULLUP);
      pinMode(sing_logic_out[i], INPUT_PULLUP);
    }
    else {
      pinMode(logic_out[i], INPUT_PULLUP);
      continue;
    }
  }
  
  pinMode(analog_logic, INPUT);   // external pulldown is set
  pinMode(active_state[0], OUTPUT);
  pinMode(active_state[1], OUTPUT);
  for(int i = 0 ; i < 4; i++)
    pinMode(button_pin[i], INPUT);
  pinMode(pwm_input, INPUT);  //initialize digital 3 pin to pwm_input

  pwm_input = digitalPinToInterrupt(pwm_input);       // change the pin type to the accurate pin type
  attachInterrupt(pwm_input, rising_edge, RISING);
  
  digitalWrite(active_state[0], HIGH);
  digitalWrite(active_state[1], LOW);
}


// Software implementation for analog encoder counter
void analog_ecoder(int *counter) {
  int new_val = analogRead(analog_logic);
  if(new_val != *counter) {
    Serial.println(*counter);
    *counter = new_val;
  }
}

// Software implementation for quadrature encoder counter
void dl_encoder(unsigned char *old_state, unsigned char *new_state, int channels[], int array_size, int *counter) {
  if(array_size < 3)    // indicates a single line encoder
    *new_state = ((digitalRead(channels[0]) << 1) | digitalRead(channels[1]));
  else
    *new_state = ((digitalRead(channels[1]) << 1) | digitalRead(channels[2]));
  // Serial.println(*new_state);
  if(*new_state != *old_state) {
   
    switch(*old_state) {
      case(0):
        if(*new_state == 1)
          (*counter)--;
        else if(*new_state == 2)
          (*counter)++;
        else
        break;
      case(1):
        if(*new_state == 0)
          (*counter)++;
        else if(*new_state == 3)
          (*counter)--;
        else
        break;
      case(2):
        if(*new_state == 0)
          (*counter)--;
        else if(*new_state == 3)
          (*counter)++;
        else
        break;
      case(3):
        if(*new_state == 1)
          (*counter)++;
        else if(*new_state == 2)
          (*counter)--;
        else
        break;
      default:
        break;
    }
    if((*counter >= 255) || (*counter <= -255))
      (*counter) = 0;
  }

  *old_state = *new_state;
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

void rising_edge() {
  digitalRead(pwm_input);
  time_stamp = micros();  // get current time stamp
  // note: The first calculation of duty cycle will be in error because two ton readings are needed.  After that, all is well.
  dty_ccl = time_stamp - ton;  // Duty Cycle = current rising edge time - previous rising edge time
  ton = time_stamp;        // reads the time of the rising edge
  attachInterrupt(digitalPinToInterrupt(pwm_input), falling_edge, FALLING);
}

void falling_edge() {
  toff = micros();      // reads time when falling edge is triggered
  x = (toff - ton);
  if (x > 1024)       // clip in case too large
    x = 1024;
  reading = (x)/1024;
  attachInterrupt(digitalPinToInterrupt(pwm_input), rising_edge, RISING);
}
