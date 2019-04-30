/*
 The circuit:
 * RX is digital pin 10 (connect to TX of other device)
 * TX is digital pin 11 (connect to RX of other device)
 */
#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // RX, TX

#include <NMEAGPS.h>
static NMEAGPS  gps; // This parses the GPS characters




static void use_the_data( const gps_fix & fix )
{
  //  This is the best place to do your time-consuming work, right after
  //     the RMC sentence was received.  If you do anything in "loop()",
  //     you could cause GPS characters to be lost, and you will not
  //     get a good lat/lon.
  //  For this example, we just print the lat/lon.  If you print too much,
  //     this routine will not get back to "loop()" in time to process
  //     the next set of GPS data.

  if (fix.valid.location) {
    
    Serial.print( fix.dateTime );
    Serial.print(" datetime, ");
    
    Serial.print("20");
    if ( fix.dateTime.year < 10 ) Serial.print( "0" );
    Serial.print( fix.dateTime.year );
    Serial.print("-");
    if ( fix.dateTime.month < 10 ) Serial.print( "0" );
    Serial.print( fix.dateTime.month );
    Serial.print("-");
    if ( fix.dateTime.date < 10 ) Serial.print( "0" );
    Serial.print( fix.dateTime.date );
    Serial.print(" ");
    if ( fix.dateTime.hours < 10 ) Serial.print( "0" );
    Serial.print( fix.dateTime.hours );
    Serial.print(":");
    if ( fix.dateTime.minutes < 10 ) Serial.print( "0" );
    Serial.print( fix.dateTime.minutes );
    Serial.print(":");
    if ( fix.dateTime.seconds < 10 ) Serial.print( "0" );
    Serial.print( fix.dateTime.seconds );
    Serial.print(", ");
    

    // Serial.print( fix.latitude(), 6 ); // floating-point display
    Serial.print( fix.latitudeL() ); // integer display
    Serial.print(" lat, ");
    // Serial.print( fix.longitude(), 6 ); // floating-point display
    Serial.print( fix.longitudeL() );  // integer display    
    Serial.print(" lon, ");
    Serial.print( fix.hdop );  // integer display    
    Serial.print(" hdop ");

  } else {
    // No valid location data yet!
    Serial.print( "No valid location data yet." );
  }
  Serial.println();
} 

void setup()
{
  
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  delay(1000); // allow serial to start communication
  
  Serial.print( F("Hello from Test GPS software\n") );
  Serial.print( F("GPS fix object size = ") );
  Serial.println( sizeof(gps.fix()) );
  Serial.print( F("NMEAGPS object size = ") );
  Serial.println( sizeof(gps) );

  #ifdef NMEAGPS_NO_MERGING
    Serial.println( F("Only displaying data from xxRMC sentences.\n  Other sentences may be parsed, but their data will not be displayed.") );
  #endif
  Serial.flush();

  // begin GPS
  mySerial.begin(9600);

  
  Serial.print( F("\nDisplaying GPS serial data ") );
  unsigned long gps_listen_startTime = millis(); 
  unsigned long gps_timeout = 3;  // sec
  Serial.print( F("for ") );
  Serial.print( gps_timeout );
  Serial.println( F(" sec. ") );
  
  int times_without_char=0;
  while((millis() - gps_listen_startTime) < (gps_timeout * 1000L)) {
    // for debugging
    if (mySerial.available()) {
      char c = mySerial.read();
      Serial.write(c);
    }
    if(times_without_char++>200 ) {
      Serial.write(" .");
      times_without_char=0;
    }
  }
  Serial.println();
  Serial.println();
}

void loop()
{
//  // NMEAGPS
  while (gps.available( mySerial )) {
    use_the_data( gps.read() );
  }

/* 

void loop() { // run over and over
  if (mySerial.available()) {
    Serial.write(mySerial.read());
  }
  if (Serial.available()) {
    mySerial.write(Serial.read());
  }
}
  */
}
