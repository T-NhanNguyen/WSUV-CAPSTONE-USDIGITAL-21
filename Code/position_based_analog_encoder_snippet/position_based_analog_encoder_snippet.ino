     int analogPin = A3; // potentiometer wiper (middle terminal) connected to analog pin 3
                    // outside leads to ground and +5V
     int val = 0;  // variable to store the value read

     int voltage = 0; // final output

       void setup() 
       
       {
  Serial.begin(9600);           //  setup serial
}

     void loop() {
    
     val = analogRead(analogPin);  // read the input pin

     // The second picture is about because the input votlage between 0 - 5V are corresponding interger 0 - 1023 in Arudnio . Thus, all i did above are trying to get the correct value betweeen 0 -1023. 
     
 
   float voltage = val*0.00488;
    //   when I get the ccorrect intergers, Ill change it to the output voltage with the formula below.

                 // 1024(intergers)  * x = 5
                             //         x= 0.00488. 

                                      
    
   Serial.println(voltage);          // debug the output voltagevalue
}
