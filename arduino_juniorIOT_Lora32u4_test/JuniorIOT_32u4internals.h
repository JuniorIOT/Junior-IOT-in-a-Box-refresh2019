
///////////////////////////////////////////////
//  some other measurements
///////////////////////////////////////////

#define VBATPIN A9

long readVbat() {
  long result;
  float measuredvbat = analogRead(VBATPIN);
    // devide by 1024 to convert to voltage
    // we divided by 2 using 2x 100M Ohm, so multiply x2
    // Multiply by 3.3V, our reference voltage
    // *1000 to get milliVolt
  measuredvbat *= 6600. / 1024.;         
  result = measuredvbat;
  Serial.print(F("  Vbat=")); 
  Serial.print(result);
  Serial.println(F(" milliVolt"));
  return result;
}

//------------------------------

void internals_measure() {  
  long vbat = readVbat();
  uint8_t vbat_bin = vbat/20 ;  // rescale 0-5100 milli volt into 0 - 255 values and make sure it is not bigger than one byte
  Serial.print(F("  Vbat=")); Serial.print(vbat); Serial.print(F(" mV. bin=")); Serial.print(vbat_bin); Serial.println();
  myLoraWanData[internals_sendbufferStartByte + 2] = vbat_bin;
}


void internals_init () {
  // as part of init, do one measurement to load values into send buffer
  internals_measure();
}
