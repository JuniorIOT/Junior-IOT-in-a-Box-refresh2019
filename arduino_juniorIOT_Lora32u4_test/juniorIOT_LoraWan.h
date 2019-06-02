#include <lmic_slim.h>     // the really cool micro-library, to replace our 2017 LMIC which filled 99% memory
#include <Arduino.h>

int TX_COMPLETE_was_triggered = 0;  // 20170220 added to allow full controll in main Loop
unsigned long last_lora_time = millis(); // last time lorawan 
unsigned long last_check_time = millis(); // last time we checked for movement detection

uint32_t charToHex(char c){
  switch(c){
    case '0': return 0x00;     case '1': return 0x01;     case '2': return 0x02;     case '3': return 0x03; 
    case '4': return 0x04;     case '5': return 0x05;     case '6': return 0x06;     case '7': return 0x07; 
    case '8': return 0x08;     case '9': return 0x09;     case 'A': return 0x0A;     case 'B': return 0x0B; 
    case 'C': return 0x0C;     case 'D': return 0x0D;     case 'E': return 0x0E;     case 'F': return 0x0F; 
  }
}

//////////////////////////////////////////////////
// Junior IOT routines for LMIC_slim (by Denniz) for LoraWan
///////////////////////////////////////////////

void lmic_slim_init() {
  //#ifdef SHOW_DEBUG
  Serial.print(F("lmic_slim_init. t=")); Serial.println(millis());  
  //#endif
  
  spi_start();
  pinMode(SS_pin, OUTPUT);   
  pinMode(SCK_pin, OUTPUT); 
  pinMode(MOSI_pin, OUTPUT);
  digitalWrite(SCK_pin, LOW);            // SCK low
  digitalWrite(SS_pin, HIGH);            // NSS high
  delay(10);
  //writeReg(0x01, 0x08); //set RFM_95 to low freq, sleep mode
  delay(10);
  //radio_init();  // that is in the LMIC_slim library
  delay(10);
  
//  OLD style key registration
//  uint8_t appskey[sizeof(APPSKEY)];
//  uint8_t nwkskey[sizeof(NWKSKEY)];
//  memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
//  memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));

  // new style key registration
  uint8_t nwkskey[16];  
  // memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
    char *p = nwkSKey;
    for(int i = 0; i < 16; i++){
      char c1 = *p;
      p++;
      char c2 = *p;
      p++;
      uint8_t ui1 = charToHex(c1);
      uint8_t ui2 = charToHex(c2);
      uint8_t ui = ui1 << 4 | ui2;
      nwkskey[i]=ui;
    }

  // new style key registration
  uint8_t appskey[16];
  // memcpy_P(appskey, APPSKEY, sizeof(APPSKEY)); 
  p = appSKey;
  for(int i = 0; i < 16; i++){
    char c1 = *p;
    p++;
    char c2 = *p;
    p++;
    uint8_t ui1 = charToHex(c1);
    uint8_t ui2 = charToHex(c2);
    uint8_t ui = ui1 << 4 | ui2;
    appskey[i]=ui;
  }

  // new style key registration
  p = devAddr;
  uint32_t DEVADDR = ((((((charToHex(*p++) << 4 | charToHex(*p++)) << 4 | charToHex(*p++)) << 4 | charToHex(*p++)) << 4 | charToHex(*p++)) << 4 | charToHex(*p++)) << 4 | charToHex(*p++)) << 4 | charToHex(*p);
  
  Serial.print("  TTN registration:  LMIC_setSession (DEVADDR, nwkskey, appskey) \n");
  Serial.print("    DEVADDR = ");Serial.print(DEVADDR, HEX);Serial.print(" \n");
  Serial.print("    nwkskey = ");
    //Serial.print(nwkskey);
    for(int i=0;i<16;i++){if(nwkskey[i]<16)Serial.print('0');Serial.print(nwkskey[i],HEX);Serial.print(" ");} Serial.print(" \n");
  Serial.print("    appskey = ");
    //Serial.print(appskey);
    for(int i=0;i<16;i++){if(appskey[i]<16)Serial.print('0');Serial.print(appskey[i],HEX);Serial.print(" ");} Serial.print(" \n");
  Serial.println();


  
  LMIC_setSession (DEVADDR, nwkskey, appskey);
  
  LMIC_LORARegModemConfig (0x72, 0b01110100, 0x04);  // LORARegModemConfig1, LORARegModemConfig2, LORARegModemConfig3
    // USE THESE SETS FOR CORRECT LORA SPEC:
    //        0x72, 0b11000100, 0x0C  // BW=125, Coding Rate=4/5, SF12, mobile
    //        0x72, 0b10110100, 0x0C  // BW=125, Coding Rate=4/5, SF11, mobile
    //        0x72, 0b10100100, 0x04  // BW=125, Coding Rate=4/5, SF10, static
    //        0x72, 0b10010100, 0x04  // BW=125, Coding Rate=4/5, SF9, static 
    //        0x72, 0b10000100, 0x04  // BW=125, Coding Rate=4/5, SF8, static
    //        0x72, 0b01110100, 0x04  // BW=125, Coding Rate=4/5, SF7, static -> this is preferred setting for generic nodes and is the TTNmapper default
    // LORARegModemConfig1
    //       0x72 is normal BW=125 en Coding Rate=4/5, all other messages I see on our TTN basestation have BW=125 en Coding Rate=4/5
    // LORARegModemConfig2 
    //       nnnn----   Spreading Factor (bit 7..4)
    //       ----0---   TxContinuousMode =0 normal mode (bit 3)  
    //       -----1--   RxPayloadCrcOn = 1 CRC ON (bit 2)  
    //       ------00   SymbTimeout(9:8)=00 default (bit 1..0)
    // LORARegModemConfig3, bit 3 van RegModemConfig3, spec van de RFM95 zegt: bit 3: MobileNode, type: rw, Default: 0x00, Value: 0 : Use for static node
    //       0x0C  originally by Rene Harte, IOT-partners for SF11, SF12  
    //       0x04  for SF7.....SF10 
    // Airtime voor 5 bytes payload = 13 x 2^(SF-6) ms. 
    //       Number of sec per message with 30-50 bytes: roughly SF12 = 2 seconds, SF10 = 0,5 sec, SF8 = 120 msec, SF7= 60 msec. One device is allowed a total of 30 seconds per day airtime.
    //       Message frequency allowed SF12 = 15 messages per day = 1 message every 90 minutes; SF11 = 1 per 45 min; SF10 = 1 per 24 min, SF9 = 1 per 12 min, SF8 = 1 per 6 min, SF7 = 1 per 3 min roughly
    // For your received messages:
    //       RSSI  Received signal strength indicator, as low as -120 is told to be workable, but I have seen it work even lower. I get -120 from my work table. Does not change with SF.
    //       SNR   Signal to noise ratio, as low as -20dB seems workable. I get -5 from my work table.
}

void print_myLoraWanData() {
  //#ifdef SHOW_DEBUGLEVEL2
  Serial.print(F("  myLoraWanData = [")); 
  //Serial.print((char*)myLoraWanData); Serial.println("]"); Serial.print(F("                  [ "));  
  for(int i=0; i<PAYLOADSIZE; i++) {  
    if (myLoraWanData[i] < 16) Serial.print(F("0")); 
    Serial.print(myLoraWanData[i], HEX); 
    Serial.print(F(" "));  
  }  
  Serial.println("]");
  //#endif 
}

//------------------------------

void LoraWan_send() {  
  Serial.println(F("   - - - - About to send one LoraWan message. ")); 
  last_lora_time = millis();
    
  #ifdef SHOW_DEBUG
  Serial.print(F("Start: - - - - - - - - - - -- - - - doOneLoraWan. t=")); Serial.println(millis());
  print_myLoraWanData();
  #endif

  LMIC_setTxData2(myLoraWanData, PAYLOADSIZE);
  radio_init();
  delay (10);
  digitalWrite(LEDPIN, !digitalRead(LEDPIN));
  
  //#ifdef SHOW_DEBUGLEVEL2
  Serial.print(F("  txLora. t=")); Serial.println(millis());
  //#endif
  txlora();
  
  //#ifdef SHOW_DEBUGLEVEL2
  Serial.print(F("  txLora completed. t=")); Serial.println(millis());
  //#endif
  
  delay(200);           // this is a simple wait with no checking for TX Ready. Sdjust this for your SF.
                          // Airtime voor 5 bytes payload = 13 x 2^(SF-6) ms. 
                          // with 30-50 bytes: SF12 = 2 seconds, SF10 = 0,5 sec, SF8 = 120 msec, SF7= 70 msec. One device has 30 seconds per day airtime.
  //digitalWrite(LED_BUILTIN, LOW);
  
  #ifdef SHOW_DEBUGLEVEL2
  Serial.print(F("  send time delay completed. t=")); Serial.println(millis());
  #endif
  
  digitalWrite(LEDPIN, !digitalRead(LEDPIN));
  setopmode(0x00);                     // opmode SLEEP; better not tell lorawan to go to sleep before message is done
  
  #ifdef SHOW_DEBUGLEVEL2
  Serial.print(F("Completed: Do one lora. t=")); Serial.println(millis());
  #endif
}

void LoraWan_init () {
  
  Serial.print(F("\n\nLoraWan_init\nStarting LoraWan. ")); 
  lmic_slim_init();
       
  last_lora_time = millis();
  last_check_time = millis();
  
  //#ifdef SHOW_DEBUGLEVEL2
  Serial.print(F("  Send one lorawan message as part of init. t=")); Serial.println(millis());
  //#endif
  
  // as part of init, do one send
  LoraWan_send();
}
