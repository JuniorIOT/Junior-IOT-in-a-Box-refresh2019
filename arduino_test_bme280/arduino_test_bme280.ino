/*
 * Example sketch for bme280 to test BME 280 sensor and see how to put values in a message buffer
 */
#include "Seeed_BME280.h"
#include <Wire.h>
#define PAYLOADSIZE 6 //Max 40    The size of the package to be sent

BME280 bme280;
float bme280_temperature;
float bme280_pressure;
float bme280_humidity;
uint16_t temp;
uint16_t hum;
uint16_t p;
uint8_t  message[6];  // including byte[0]

void setup()
{
  Serial.begin(9600);
  if(!bme280.init()){
    Serial.println("Device error!");
  }
}


void print_message() {
  Serial.print(F("message = [")); 
  for(int i=0; i<PAYLOADSIZE; i++) {  
    if (message[i] < 16) Serial.print(F("0")); 
    Serial.print(message[i], HEX); 
    Serial.print(F(" "));  
  }  
  Serial.println("]");
}

void loop()
{  
  //get and print temperatures
  bme280_temperature = bme280.getTemperature();
  Serial.print("Temp: ");
  Serial.print(bme280_temperature);
  Serial.println("C");//The unit for  Celsius because original arduino don't support special symbols
  
  //get and print atmospheric pressure data
  bme280_pressure = bme280.getPressure();
  Serial.print("Pressure: ");
  Serial.print(bme280_pressure);
  Serial.println("Pa");

  //get and print humidity data
  bme280_humidity = bme280.getHumidity();
  Serial.print("Humidity: ");
  Serial.print(bme280_humidity);
  Serial.println("%");

  //put everything in a message buffer and print it
  temp = (bme280_temperature + 100 )*100 ;
  message[0] = temp >> 8;
  message[1] = temp;
  
  hum = (bme280_humidity) * 100 ;
  message[2] = hum >> 8;
  message[3] = hum;
   
  p = (bme280_pressure)/10 ;
  message[4] = p >> 8;
  message[5] = p;  

  print_message();

  delay(1000);
}
