/*******************************************************************************
 * Modified By DenniZr & Marco van Schagen for Junior IOT - Smart City Challenge 2018 #juniorIOTchallenge2018
 * Modified By Marco van Schagen for Junior IOT Challenge 2018
 * Modified By Ted for ICTindeWolken 2019
 *******************************************************************************/ 
 
// build options for Arduino IDE: 
//   Lora32u4            - compile as adafruit feather 32u4

// Libraries to add to Arduino IDE from our libraries subfolder
//    <lmic_slim.h>    --> \lmic_slim.zip

// definition of internals
#define LEDPIN 13 


//-------------- Message Byte Positions  ------------//

#define PAYLOADSIZE 9 //Max 40    The size of the package to be sent
uint8_t  myLoraWanData[9];  // including byte[0]


//-------------- LoraWan libraries, mappings and things ------------//
#define LORAWAN_TX_INTERVAL_MAX 240   // seconds between LoraWan messages

#include "keys.h"          // the personal keys to identify our own nodes, in a file outside GITHUB
#include "juniorIOT_LoraWan.h" // this is where all the LoraWan code and libraries are defined



void setup() {  
  pinMode(LEDPIN, OUTPUT);
  
  delay(2500);     // Give time to the ATMega32u4 port to wake up and be recognized by the OS.
  Serial.begin(115200); 
  Serial.println(F("\nStarting device.")); 
  Serial.print(F("Setup() started. t=")); Serial.println(millis());
  // once all values have been initialized, init Lora and send message to TTN
  LoraWan_init();  // --> init and also send one message    
  Serial.print(F("Setup() completed. t=")); Serial.println(millis());
}


void loop() {
  Serial.print(F("\n==== Loop starts. t=")); Serial.println(millis());
  digitalWrite(LEDPIN, !digitalRead(LEDPIN)); 

  // write values in message byte by byte and show values on serial monitor (in Hex)
  myLoraWanData[0] = 80;
  myLoraWanData[1] = 79;
  myLoraWanData[2] = 69;
  myLoraWanData[3] = 80;
  myLoraWanData[4] = 32;
  myLoraWanData[5] = 58;
  myLoraWanData[6] = 45;
  myLoraWanData[7] = 41;
  myLoraWanData[8] = 32;
  print_myLoraWanData();

  //send the message
  lmic_slim_init();
  LoraWan_send();    

  delay(LORAWAN_TX_INTERVAL_MAX * 1000L); 
      
}
