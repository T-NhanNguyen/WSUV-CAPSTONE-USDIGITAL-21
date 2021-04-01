#include <SPI.h>
#include <util/delay.h>            // Delay functions
#include <SoftwareSerial.h>        // Serial Library
#include "Arduino.h"               // Arduino

const int cs_1 = 6;
const int cs_2 = 12;

volatile long test_read1;
volatile long test_read2;

/*
SPCR configuration
CPOL = 0, CPHA = 0, Mode0
DORD = 0, MSB first
MSTR = 1, Master
SPE = 1, SPI enabled
SCK frequency = fosc/4
*/

//Count modes
#define NQUAD 0x00 //non-quadrature mode
#define QUADRX1 0x01 //X1 quadrature mode
#define QUADRX2 0x02 //X2 quadrature mode
#define QUADRX4 0x03 //X4 quadrature mode
//Running modes
#define FREE_RUN 0x00
#define SINGE_CYCLE 0x04
#define RANGE_LIMIT 0x08
#define MODULO_N 0x0C
//Index modes
#define DISABLE_INDX 0x00 //index_disabled
#define INDX_LOADC 0x10 //index_load_CNTR
#define INDX_RESETC 0x20 //index_rest_CNTR
#define INDX_LOADO 0x30 //index_load_OL
#define ASYNCH_INDX 0x00 //asynchronous index
#define SYNCH_INDX 0x80 //synchronous index
//Clock filter modes
#define FILTER_1 0x00 //filter clock frequncy division factor 1
#define FILTER_2 0x80 //filter clock frequncy division factor 2
/* **MDR1 configuration data; any of these***
 ***data segments can be ORed together***/
//Flag modes
#define NO_FLAGS 0x00 //all flags disabled
#define IDX_FLAG 0x10 //IDX flag
#define CMP_FLAG 0x20 //CMP flag
#define BW_FLAG 0x40 //BW flag
#define CY_FLAG 0x80 //CY flag
//1 to 4 bytes data-width
#define BYTE_4 0x00 //four byte mode
#define BYTE_3 0x01 //three byte mode
#define BYTE_2 0x02 //two byte mode
#define BYTE_1 0x03 //one byte mode
//Enable/disable counter
#define EN_CNTR 0x00 //counting enabled
#define DIS_CNTR 0x04 //counting disabled

/* LS7366R op-code list */
#define CLR_MDR0 0x08
#define CLR_MDR1 0x10
#define CLR_CNTR 0x20
#define CLR_STR 0x30
#define READ_MDR0 0x48
#define READ_MDR1 0x50

#define READ_CNTR 0x60
#define READ_OTR 0x68
#define READ_STR 0x70
#define WRITE_MDR1 0x90
#define WRITE_MDR0 0x88
#define WRITE_DTR 0x98
#define LOAD_CNTR 0xE0
#define LOAD_OTR 0xE4

uint8_t ls7366r_recieve(uint8_t device, uint8_t op_code) {
  uint8_t data = 0;

  // Serial.print("opcode for read: "); Serial.println(op_code);
  digitalWrite(device, LOW);
  SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0)); 


  SPI.transfer(op_code);          // send out opcode
  data |= SPI.transfer(0);

  SPI.endTransaction();
  digitalWrite(device, HIGH);
  return data;
}

void ls7366r_write(uint8_t device, uint8_t op_code, uint8_t data) {
  
  // Serial.print("opcode and data for write: "); Serial.print(op_code);
  // Serial.print(" "); Serial.println(data);
  digitalWrite(device, LOW);
  SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0)); 

  SPI.transfer(op_code);            // required opcode
  SPI.transfer(data);

  SPI.endTransaction();
  digitalWrite(device, HIGH);
}

// "device" param corresponds to the CS pin assigned to the following device
void ls7366r_init(uint8_t device) {
  pinMode(device, OUTPUT);
  
  digitalWrite(device, HIGH); // cs default high
  
  // NOTE: LS7366R opperates faster than arduino
  SPI.begin();

  uint32_t mdr1_preset  = (IDX_FLAG|CMP_FLAG|BYTE_4|EN_CNTR);
  // uint32_t mdr0_preset  = (FILTER_2|SYNCH_INDX|INDX_LOADC|FREE_RUN|QUADRX4);
  uint32_t mdr0_preset  = 0x01;

  // init encoder 1
  ls7366r_write(device, WRITE_MDR1, mdr1_preset);   // reg involving counters
  ls7366r_write(device, WRITE_MDR0, mdr0_preset);   // reg involve operation
  ls7366r_write(device, CLR_CNTR, 0);               // where encoder count is stored
  ls7366r_write(device, CLR_STR, 0);                // reg involves count related status information
  ls7366r_write(device, LOAD_OTR, 0);
  // verifying the decoder by comparing reg values
  error_check(ls7366r_recieve(device, READ_MDR1), mdr1_preset, __LINE__);
  error_check(ls7366r_recieve(device, READ_MDR0), mdr0_preset, __LINE__);
  // error_check(ls7366r_read_cntr(device), 0, __LINE__);

  digitalWrite(device, HIGH); // cs default high
}

long ls7366r_read_cntr(uint8_t device) {
  uint8_t byte_mode = ls7366r_recieve(device, READ_MDR1); 
  uint8_t data_length = 0;
  long data = 0;
  byte_mode &= 3;       // determine which byte counter mode from LSB
  switch(byte_mode) {   // assign the length for read loop
    case 0:
      data_length = 4;
      break;
    case 1:
      data_length = 3;
      break;
    case 2:
      data_length = 2;
      break;
    case 3:
      data_length = 1;
      break;
  }

  digitalWrite(device, LOW);
  SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0)); 
  
  // perform bit shifts while appending the next byte
  if(data_length == 1) {
    data |= ls7366r_recieve(device, READ_CNTR);
  }
  else {
    for(int i = 0; i < (data_length - 1); i++) {
      data |= ls7366r_recieve(device, READ_CNTR);      // reads in a byte at a time
      data <<= 8;
    }
    data |= ls7366r_recieve(device, READ_CNTR);        // finish the LS-byte
  }
  
  
  digitalWrite(device, HIGH);
  SPI.endTransaction();

  return data;
}

void print_concatstr(char *sentence, long int value) {
  Serial.print(sentence);
  Serial.print(" ");
  Serial.println(value, HEX);
}

void error_check(long int compare_val1, long int compare_val2, uint32_t error_line) {
  if(compare_val1 != compare_val2) {
    Serial.print(F("Failed validity check at:"));
    Serial.println(error_line);

    while(1) {
      digitalWrite(A1, HIGH);
      delay(500);
      digitalWrite(A1, LOW);
      delay(500);
    }
  }
} 
void setup() {
  pinMode(A1, OUTPUT);    // debug_led
  Serial.begin(9600);   //open serial port at 9600 bps
  delay(2000);
  ls7366r_init(cs_1);
  ls7366r_init(cs_2);
}

void loop() {
  test_read1 = ls7366r_read_cntr(cs_2);
  print_concatstr("Read CNTR 1:", test_read1);
}


