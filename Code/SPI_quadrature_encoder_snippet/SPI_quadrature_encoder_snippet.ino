#include <SPI.h>
#include <util/delay.h>            // Delay functions
#include <SoftwareSerial.h>        // Serial Library
#include "Arduino.h"               // Arduino

const int cs_1 = 6;
const int cs_2 = 12;

volatile long int test_read1;
volatile long int test_read2;

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

long int ls7366r_read_cntr(uint8_t device);

uint8_t ls7366r_recieve(uint8_t device, uint8_t op_code) {
  uint8_t data = 0;

  // Serial.print("opcode for read: "); Serial.println(op_code);
// *** slowing down the SPI clock to within capabilities of 7366 device (< 4MHz)
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0)); 
// *** switched enable/disable signal activation to within the SPI activation area, per SPI library recommendation
  digitalWrite(device, LOW);


  SPI.transfer(op_code);          // send out opcode
  data |= SPI.transfer(0);

  digitalWrite(device, HIGH);
  SPI.endTransaction();
  return data;
}

void ls7366r_write(uint8_t device, uint8_t op_code, uint8_t data) {
  
// *** SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));
// *** slowing down the SPI clock to within capabilities of 7366 device (< 4MHz)
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));

  // Serial.print("opcode and data for write: "); Serial.print(op_code);
  // Serial.print(" "); Serial.println(data);
// *** switched enable/disable signal activation to within the SPI activation area, per SPI library recommendation
  digitalWrite(device, LOW);

  SPI.transfer(op_code);            // required opcode
  SPI.transfer(data);

  digitalWrite(device, HIGH);
  SPI.endTransaction();
}

// "device" param corresponds to the CS pin assigned to the following device
void ls7366r_init(uint8_t device) {
  pinMode(device, OUTPUT);
  
  digitalWrite(device, HIGH); // cs default high
  
  // NOTE: LS7366R opperates faster than arduino
  SPI.begin();

  uint32_t mdr1_preset  = (BYTE_2|EN_CNTR);
  uint32_t mdr0_preset  = (FILTER_2|DISABLE_INDX|FREE_RUN|QUADRX4);
  
  // init encoder 1
  ls7366r_write(device, WRITE_MDR1, mdr1_preset);   // reg involving counters
  ls7366r_write(device, WRITE_MDR0, mdr0_preset);   // reg involve operation
  ls7366r_write(device, CLR_CNTR, 0);               // where encoder count is stored
  ls7366r_write(device, CLR_STR, 0);                // reg involves count related status information

  // verifying the decoder by comparing reg values
  if( (ls7366r_recieve(device, READ_MDR1) != mdr1_preset) ||
      (ls7366r_recieve(device, READ_MDR0) != mdr0_preset)  )
    {
        Serial.print(F("Failed validity check"));
        digitalWrite(A1, HIGH);
        while(1);
    }

  digitalWrite(device, HIGH); // cs default high
}

long int ls7366r_read_cntr(uint8_t device) {
  uint8_t byte_mode = ls7366r_recieve(device, READ_MDR1); 
  uint8_t data_length = 0;
  long int data = 0;
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

  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0)); 
  digitalWrite(device, LOW);
  
  SPI.transfer(READ_CNTR);          // send out opcode
  
  // perform bit shifts while appending the next byte
  for(int i = 0; i < (data_length - 1); i++) {
    data |= SPI.transfer(0);
// *** changed bit shift from 4 to 8
    data <<= 8;
  }
  data |= SPI.transfer(0);

  digitalWrite(device, HIGH);
  SPI.endTransaction();
  
  return data;
}

void print_concatstr(char *sentence, int value) {
  Serial.print(sentence);
  Serial.print(" ");
  Serial.println(value);
}


void setup() {
  pinMode(A1, OUTPUT);  // debug_led
  digitalWrite(A1, LOW);
  Serial.begin(9600);   //open serial port at 9600 bps
  // delay(2000);
  ls7366r_init(cs_1);
  ls7366r_init(cs_2);
}

void loop() {
  test_read1 = ls7366r_read_cntr(cs_1);
  test_read2 = ls7366r_read_cntr(cs_2);
  print_concatstr("Read CNTR 1:", int(test_read1));
  print_concatstr("Read CNTR 2:", int(test_read2));

/*
  test_read1 = ls7366r_recieve(cs_1 ,READ_STR);
  test_read2 = ls7366r_recieve(cs_2 ,READ_STR);
  print_concatstr("READ_STR 1:", test_read1);
  print_concatstr("READ_STR 2:", test_read2);

  test_read1 = ls7366r_recieve(cs_1 ,READ_OTR);
  test_read2 = ls7366r_recieve(cs_2 ,READ_OTR);
  print_concatstr("READ_OTR 1:", test_read1);
  print_concatstr("READ_OTR 2:", test_read2);
*/

//  delay(1000);
}
