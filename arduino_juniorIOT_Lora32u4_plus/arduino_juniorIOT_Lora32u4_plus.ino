/*******************************************************************************
 * Modified By DenniZr & Marco van Schagen for Junior IOT - Smart City Challenge 2018 #juniorIOTchallenge2018
 * Modified By Marco van Schagen for Junior IOT Challenge 2018
 * Modified By Ted for ICTindeWolken 2019
 *******************************************************************************/ 
 
// build options for Arduino IDE: 
//   Lora32u4            - compile as adafruit feather 32u4

// Libraries to add to Arduino IDE from our libraries subfolder C:\arduino_port\GitHub\Junior-IOT-in-a-box-Project\libraries\ 
//    <NeoSWSerial.h>  --> \NeoSWSerial-master-DamiaBranch.zip
//    <NMEAGPS.h>      --> \NeoGPS-master.zip  --> and adjust them according to lines 45- below
//    <lmic_slim.h>    --> \lmic_slim.zip

// definition of internals
#define LEDPIN 13 

//-------------- Message Byte Positions  ------------//
#define GPS_sendbufferStartByte 0 //9 bytes
#define BME280_sendbufferStartByte 26 //6 bytes

#define PAYLOADSIZE 32 //Max 40    The size of the package to be sent
uint8_t  myLoraWanData[32];  // including byte[0]
unsigned long datetime_gps=0;

//-------------- generic debug stuff ------------//
//#define SHOW_DEBUG           // if SHOW_DEBUG is defined, some code is added to display some basic SHOW_DEBUG info
//#define SHOW_DEBUGLEVEL2     // if SHOW_DEBUGLEVEL2 is defined, some code is added to display deeper SHOW_DEBUG info

//-------------- LoraWan libraries, mappings and things ------------//
#define LORAWAN_TX_INTERVAL_MIN 15  // seconds between LoraWan messages if change in values is detected
#define LORAWAN_TX_INTERVAL_MAX 240   // seconds between LoraWan messages
#define TXTRIGGER_gps_movement 8000 // will send if Min interval reached and GPS movement > value
    // gps coordinates in Long value means 52.6324510 translates into 526324510
    // 5000 trigger value means 25-30 meters

#include "keys.h"          // the personal keys to identify our own nodes, in a file outside GITHUB
#include "juniorIOT_LoraWan.h" // this is where all the LoraWan code and libraries are defined

//-------------- GPS things ------------//
#define GPS_FIX_HDOP   // to prevent eror: 'const class gps_fix' has no member named 'hdop'
#define GPS_RXD_PIN 10  // TXD on gps to rx arduino
#define GPS_TXD_PIN 11    // RXD on gps to tx arduino
#include "juniorIOT_GPS.h"  // this is where all GPS code and libraries are defined

//----------sensor things -------//
#include <Wire.h>
#include "JuniorIOT_BME280.h"


void setup() {  
  pinMode(LEDPIN, OUTPUT);
  
  delay(2500);     // Give time to the ATMega32u4 port to wake up and be recognized by the OS.
  Serial.begin(115200);   // whether 9600 or 115200; the gps feed shows repeated char and cannot be interpreted, setting high value to release system time
    
  Serial.println(F("\nStarting device.")); 
  Serial.print(F("Setup() started. t=")); Serial.println(millis());

  // initialize all sensors and things
  GPS_init();
  BME280_init();
  // once all values have been initialized, init Lora and send message to TTN
  LoraWan_init();  // --> init and also send one message 
    
  Serial.print(F("Setup() completed. t=")); Serial.println(millis());
}


void loop() {
  Serial.print(F("\n==== Loop starts. t=")); Serial.println(millis());
  digitalWrite(LEDPIN, !digitalRead(LEDPIN)); 

  // wait till we want to re-check the measurements
  while((millis() - last_check_time) < (LORAWAN_TX_INTERVAL_MIN * 1000L)) {
    delay(5000);   
    Serial.print(F("."));
  }
  Serial.println();
    
  // re-check all measurements
  GPS_measure();  // also gets current datetime
  Serial.print(F("\nRedo all measurements. t=")); Serial.println(millis());
  BME280_measure();
  

  ////////// Now CHECK IF we need to send a LORAWAN update to the world  ///////////
  // switch the LMIC antenna to LoraWan mode
  if ( (millis() - last_lora_time) > ((LORAWAN_TX_INTERVAL_MAX) * 1000L)
      || (l_lat_movement > TXTRIGGER_gps_movement) 
      || (l_lon_movement > TXTRIGGER_gps_movement) ) {
    Serial.print(F("   - - - - About to send one LoraWan. ")); 
    last_lora_time = millis();
    lmic_slim_init();
    LoraWan_send(); 
  
  } else {
    Serial.print(F("   - Not sending. ")); 
  }
  last_check_time = millis();
  
  Serial.print(F(" t=")); Serial.println(millis());
  Serial.print(F("     l_lat=")); Serial.print(l_lat);
  Serial.print(F("; l_lat_hist=")); Serial.print(l_lat_hist);
  Serial.print(F("; l_lat_movement=")); Serial.println(l_lat_movement);
  Serial.print(F("     l_lon=")); Serial.print(l_lon);
  Serial.print(F("; l_lon_hist=")); Serial.print(l_lon_hist);      
  Serial.print(F("; l_lon_movement=")); Serial.println(l_lon_movement);
      
  /////////// Loop again  //////////////
  #ifdef SHOW_DEBUGLEVEL2
  Serial.println(F("  End of loop. t=")); Serial.println(millis());
  #endif
}
