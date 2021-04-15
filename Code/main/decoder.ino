#include "decoder_macros.h"
/*
SPCR configuration
CPOL = 0, CPHA = 0, Mode0
DORD = 0, MSB first
MSTR = 1, Master
SPE = 1, SPI enabled
SCK frequency = fosc/4
*/

uint8_t ls7366r_recieve(uint8_t device, uint8_t op_code) {
  uint8_t data = 0;
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0)); 
  digitalWrite(device, LOW);


  SPI.transfer(op_code);          // send out opcode
  data |= SPI.transfer(0);

  digitalWrite(device, HIGH);
  SPI.endTransaction();
  return data;
}

void ls7366r_write(uint8_t device, uint8_t op_code, uint8_t data) {
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
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
