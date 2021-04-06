int input = 3;  //set input pin to digital pin 3
volatile unsigned long ton = 0;   // set initial times on to 0
volatile unsigned long toff = 0;  // set inital off time to 0 
volatile unsigned long time_stamp;  // time stamp
volatile int x = 0;       // set initial position x reading as 0
volatile int dty_ccl = 0;     // set initial duty cycle to 0
volatile unsigned int reading = 0;

void setup() {
 Serial.begin(9600);  //open serial port at 9600 bps
 pinMode(input, INPUT);  //initialize digital 3 pin to input
 attachInterrupt(digitalPinToInterrupt(input), rising_edge, RISING);  //set initial interrupt to input pin for rising edge trigger
}

void loop() {
  Serial.println(reading);
}

void rising_edge() {
  digitalRead(input);
  time_stamp = micros();  // get current time stamp
  // note: The first calculation of duty cycle will be in error because two ton readings are needed.  After that, all is well.
  dty_ccl = time_stamp - ton;  // Duty Cycle = current rising edge time - previous rising edge time
  ton = time_stamp;        // reads the time of the rising edge
  attachInterrupt(digitalPinToInterrupt(input), falling_edge, FALLING);
}

void falling_edge() {
  toff = micros();      // reads time when falling edge is triggered
  x = (toff - ton);
  if (x > 1024)       // clip in case too large
    x = 1024;

  reading = x/2.844;
  attachInterrupt(digitalPinToInterrupt(input), rising_edge, RISING);
}
