/*  Button and internal led   */
#define btnPin  A0 

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  
  // initialize pin A0 as an input with pullup.
  pinMode(btnPin, INPUT_PULLUP);  // 20k internal pullup
}

// the loop function runs over and over again forever
void loop() {
  
  int btnValue = analogRead(btnPin);   // interestingly, we replace the button by a potmeter or even ea light switch
  if (btnValue>100) {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  } else {
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  }
  
  delay(100);      
}
