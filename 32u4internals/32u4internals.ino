#include "32u4internals.h"

void setup() {  
  pinMode(LEDPIN, OUTPUT);
  
  delay(2500);     // Give time to the ATMega32u4 port to wake up and be recognized by the OS.
  Serial.begin(115200);   // whether 9600 or 115200; the gps feed shows repeated char and cannot be interpreted, setting high value to release system time
    
  Serial.println(F("\nStarting device.")); 
  Serial.print(F("Setup() started. t=")); Serial.println(millis());
  internals_init();   
  Serial.print(F("Setup() completed. t=")); Serial.println(millis());
}

void loop() {
  Serial.print(F("\n==== Loop starts. t=")); Serial.println(millis());
  digitalWrite(LEDPIN, !digitalRead(LEDPIN)); 
  delay(2500);
  Serial.print(F("\nRedo all measurements. t=")); Serial.println(millis());
  internals_measure();

}
