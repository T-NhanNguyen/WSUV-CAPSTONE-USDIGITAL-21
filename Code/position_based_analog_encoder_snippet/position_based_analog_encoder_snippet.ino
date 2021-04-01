//let potentiometer is connected at analog pin 4:

volatile int val = 0;
//variable to store the value read
void setup () 
{
 Serial.begin(9600);
 pinMode(A1, INPUT);
}

void loop () 
{
    //Used to read the input pin
     Serial.println(analogRead(A1));
     delay(1000);
}