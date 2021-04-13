//let potentiometer is connected at analog pin 4:

volatile int val = 0;
//variable to store the value read
void setup () 
{
 Serial.begin(9600);
 pinMode(A5, INPUT);
}

void loop () 
{
    //Used to read the input pin
   int value = map(analogRead(A5), 0, 670, 0, 100);
     Serial.println(value);
   //  delay(1000);
}
