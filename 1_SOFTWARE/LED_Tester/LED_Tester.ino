/*
  Blink for TemperatUHR
  Demo Code to proof the LEDs as working.
  Written by Paul Goldschmidt, 2022-08-27.
*/

// the setup function runs once when you press reset or power the board
void setup() {
  pinMode(D6, OUTPUT); // RED as OUTPUT
  pinMode(D7, OUTPUT); // BLUE as OUTPUT
  pinMode(D8, OUTPUT); // GREEN as OUTPUT
  digitalWrite(D6, HIGH);   // turn all LED off (HIGH is the voltage level, LEDs turn on when logic level turns LOW)            
  digitalWrite(D7, HIGH);                 
  digitalWrite(D8, HIGH);                   
}

// the loop function runs over and over again forever
void loop() {                 
  digitalWrite(D6, LOW);    // turn the red LED on by making the voltage LOW
  delay(1000);              // wait for a second
  digitalWrite(D6, HIGH);   // turn all LED off (HIGH is the voltage level, LEDs turn on when logic level turns LOW)    
  digitalWrite(D8, LOW);    // turn the green LED on by making the voltage LOW
  delay(1000);              // wait for a second
  digitalWrite(D8, HIGH); 
  digitalWrite(D7, LOW);    // turn the blue LED on by making the voltage LOW
  delay(1000);              // wait for a second
  digitalWrite(D7, HIGH); 
  digitalWrite(D6, LOW);   // turn all LEDs on         
  digitalWrite(D7, LOW);                 
  digitalWrite(D8, LOW);   
  delay(2000);   
  digitalWrite(D6, HIGH);   // turn all LEDs off (HIGH is the voltage level, LEDs turn on when logic level turns LOW)            
  digitalWrite(D7, HIGH);                 
  digitalWrite(D8, HIGH);     
}
