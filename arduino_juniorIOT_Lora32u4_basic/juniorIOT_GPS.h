
#include <NeoSWSerial.h>  //  We now use NeoSWSerial for lower footprint end better performance than SoftwareSerial
  // an issue with Leonardo-types is fixed in branch, yet to be merged into main version library. So you may need to remove all your NeoSWSerial libraries and add \libraries\NeoSWSerial-master-DamiaBranch.zip
NeoSWSerial ss(GPS_RXD_PIN, GPS_TXD_PIN);

// You must enable/change these lines in NMEAGPS_cfg.h:
//          #define NMEAGPS_PARSE_GLL  // juniorIOTchallenge2018
//          #define LAST_SENTENCE_IN_INTERVAL NMEAGPS::NMEA_GLL  // juniorIOTchallenge2018
// and in GPSfix_cfg.h:         
//          #define GPS_FIX_HDOP  // juniorIOTchallenge2018
#include <NMEAGPS.h>       // We now use NmeaGps (or NeoGps) as it understands newer GNSS (default reads only GGA, RMC)
static NMEAGPS gps;    // This parses the GPS characters

long gps_fix_count = 0;
long gps_nofix_count = 0;
unsigned long gps_last_time = millis();
//unsigned long gps_gets_time = 5000;



//--------------Table of contents------------//
void doGPS();
void put_gpsvalues_into_sendbuffer();
void gps_init();
void gps_setStrings();
////--------------/Table of contents------------//



void print_encoded_dt(unsigned long datetime_gps) {
//  datetime_gps = y << 26 | mo << 22 | d << 17 | h << 12 | m < 6 | s ;

  Serial.print(F(" print_encoded_dt ")); Serial.print(F(" ")); 
  Serial.print( (datetime_gps >> 26) + 2000 ); Serial.print(F("-")); 
  Serial.print( (datetime_gps >> 22) & 15); Serial.print(F("-")); 
  Serial.print( (datetime_gps >> 17) & 31); Serial.print(F(" ")); 
  Serial.print( (datetime_gps >> 12) & 31); Serial.print(F(":")); 
  Serial.print( (datetime_gps >> 6) & 63); Serial.print(F(":")); 
  Serial.print( datetime_gps & 63);  Serial.println(F(" ")); //Serial.println( datetime_gps, BIN );
}

//////////////////////////////////////////////////////////
//// Junior IOT routines for gps
////////////////////////////////////////////
long l_lat, l_lon, l_alt, l_lat_hist, l_lon_hist, l_lat_movement, l_lon_movement;
int hdopNumber; 

void put_gpsvalues_into_sendbuffer() {
  #ifdef SHOW_DEBUG
  Serial.println(F("Start: put_gpsvalues_into_sendbuffer"));
  
  #endif
  //   GPS reading = Satellite time hh:mm:18, lat 526326595, lon 47384133, alt 21, hdop 990, sat count = 12
  // With the larger precision in LONG values in NMEAGPS, 
  //    when rescaling, the values exceed the range for type LONG  -2.147.483.648 .. 2.147.483.647
  //    so we need to use DOUBLE with 15 digits precision, not preferred is FLOAT with 7 digits
  //    our values such as 526326595 have 9 digits
    
  const double shift_lat     =    90. * 10000000.;                 // range shift from -90..90 into 0..180, note: 
                                                                 //      NMEAGPS long lat&lon are degree values * 10.000.000
                                                                 //      TynyGPS long lat&lon are degree values * 1.000.000
  const double max_old_lat   =   180. * 10000000.;                 // max value for lat is now 180
  const double max_3byte     =         16777215.;                   // max value that fits in 3 bytes
  double lat_DOUBLE         = l_lat;                              // put 4byte LONG into a more precise floating point to prevent rounding during calcs 
  lat_DOUBLE = (lat_DOUBLE + shift_lat) * max_3byte / max_old_lat; // rescale into 3 byte integer range
  uint32_t LatitudeBinary  = lat_DOUBLE;                          // clips off anything after the decimal point    
  const double shift_lon     =   180. * 10000000.;                 // range shift from -180..180 into 0..360
  const double max_old_lon   = 360. * 10000000.;                   // max value longitude is now 360, note the value is too big for Long type
  double lon_DOUBLE = l_lon;                                      // put the 4byte LONG into a precise floating point memory space
  lon_DOUBLE = (lon_DOUBLE + shift_lon) * max_3byte / max_old_lon; // rescale into 3 byte integer range
  uint32_t LongitudeBinary = lon_DOUBLE;                          // clips off anything after the decimal point  
  uint16_t altitudeBinary  = l_alt    ;                          // we want altitudeGps in meters, note:
                                                                 //      NMEAGPS alt.whole is meter value ---> checked in Poznan, no need to divide by 10
                                                                 //      TynyGPS long alt is meter value * 100
  if (l_alt<0) altitudeBinary=0;                                 // unsigned int wil not allow negative values and warps them to huge number  
  uint8_t HdopBinary = hdopNumber/100;                           // we want horizontal dillution, good is 2..5, poor is >20. Note:
                                                                 //      NMEAGPS outputs an indoor value of 600..1000. Let's divide by 100
                                                                 //      from TinyGPS horizontal dilution of precision in 100ths? We succesfully divided by 10
                                                                 //      TinyGPSplus seems the same in 100ths as per MNEMA string. We succesfully divided by 10
  
//  Serial.print(F("  shift_lat = "));Serial.println(shift_lat);
//  Serial.print(F("  max_old_lat = "));Serial.println(max_old_lat);
//  Serial.print(F("  max_3byte = "));Serial.println(max_3byte);
//  Serial.print(F("  l_lat = "));Serial.println(l_lat);
//  Serial.print(F("  lat_float = "));Serial.println(lat_float);
//  Serial.print(F("  LatitudeBinary = "));Serial.println(LatitudeBinary);
//  
//  Serial.print(F("\n  shift_lon = "));Serial.println(shift_lon);
//  Serial.print(F("  max_old_lon = "));Serial.println(max_old_lon);
//  Serial.print(F("  l_lon = "));Serial.println(l_lon);
//  Serial.print(F("  lon_float = "));Serial.println(lon_float);
//  Serial.print(F("  LongitudeBinary = "));Serial.println(LongitudeBinary);
//  
//  Serial.print(F("\n  l_alt = "));Serial.println(l_alt);
//  Serial.print(F("  altitudeBinary = "));Serial.println(altitudeBinary);
//  
//  Serial.print(F("\n  hdopNumber = "));Serial.println(hdopNumber);
//  Serial.print(F("  HdopBinary = "));Serial.println(HdopBinary);
  
  myLoraWanData[GPS_sendbufferStartByte + 0] = LatitudeBinary >> 16;
  myLoraWanData[GPS_sendbufferStartByte + 1] = LatitudeBinary >> 8;
  myLoraWanData[GPS_sendbufferStartByte + 2] = LatitudeBinary;
  myLoraWanData[GPS_sendbufferStartByte + 3] = LongitudeBinary >> 16;
  myLoraWanData[GPS_sendbufferStartByte + 4] = LongitudeBinary >> 8;
  myLoraWanData[GPS_sendbufferStartByte + 5] = LongitudeBinary;
  // altitudeGps in meters into unsigned int
  myLoraWanData[GPS_sendbufferStartByte + 6] = altitudeBinary >> 8;
  myLoraWanData[GPS_sendbufferStartByte + 7] = altitudeBinary;
  // hdop in tenths of meter
  myLoraWanData[GPS_sendbufferStartByte + 8] = HdopBinary;

  print_myLoraWanData();
  
//  Dummy satellite values: time hh:mm:5, lat 52632400, lon 4738800, alt 678, hdop 2345, sat count = 12
//       0  1  2  3  4  5  6  7  8   
//     [87 7C 49 80 56 44 02 A6 17 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  .. ]

}

bool process_gps_datastream(const gps_fix & fix) {   // constant pointer to fix object
  //unsigned long age; 
  //bool GPS_values_are_valid = true;

  if (fix.valid.location) Serial.print(F(" (valid location) ")); 
  if (fix.valid.date) Serial.print(F(" (valid date) ")); 
  if (fix.valid.time) Serial.print(F(" (valid time) ")); 
  
  // read date time from gps and convert to 4 bytes, where bits are: 6 Y-2000, 4 M, 5 D, 5 H, 6 M, 6 S
  unsigned long  y = fix.dateTime.year;    // 18
  unsigned long  mo = fix.dateTime.month;  // 12 
  unsigned long  d = fix.dateTime.date;    // 31
  unsigned long  h = fix.dateTime.hours;   // 24
  unsigned long  m = fix.dateTime.minutes; // 60
  unsigned long  s = fix.dateTime.seconds; // 60
  //      0000 0000  0000 0000  0000 0000  0000 0000
  //      yyyy yymm  mmdd dddh  hhhh mmmm  mmss ssss
  //      3332 2222  2222 2111  1111 111 
  //      2109 8765  4321 0987  6543 2109  8765 4321
  datetime_gps = y << 26 | mo << 22 | d << 17 | h << 12 | m << 6 | s ;
  print_encoded_dt(datetime_gps);
  
  //if (fix.valid.location && fix.dateTime.seconds > 0) {   
  if (fix.valid.location ) {    
    l_lat = fix.latitudeL();
    l_lon = fix.longitudeL();
    l_alt = fix.alt.whole;
    //l_alt = fix.altitude_cm;
    hdopNumber = fix.hdop;
 
    
    // serial print commands will take time and may affect the gps read
    #ifdef SHOW_DEBUGLEVEL2
    Serial.print(F("  Satellite time hh:mm:")); Serial.print( fix.dateTime.seconds ); Serial.print(F(", "));
    Serial.print(F("lat ")); Serial.print( l_lat  ); Serial.print(F(", "));
    Serial.print(F("lon ")); Serial.print( l_lon ); Serial.print(F(", "));
    Serial.print(F("alt ")); Serial.print( l_alt ); Serial.print(F(", "));
    Serial.print(F("hdop ")); Serial.print( hdopNumber ); Serial.print(F(", "));
    Serial.print(F("sat = ")); Serial.print( fix.satellites );
    Serial.print( fix.dateTime ); Serial.print(" datetime, ");    
    //Serial.print( fix.latitude(), 6 ); // floating-point display
    //Serial.print( fix.longitude(), 6 ); // floating-point display
    //   Satellite time hh:mm:18, lat 526326595, lon 47384133, alt 21, hdop 990, sat count = 12

    
    Serial.println();
    #endif
    return true;
  } else {
    //#ifdef SHOW_DEBUG
    Serial.print( "(no valid location) " );
    //#endif
    return false;
  }  
}


void doGPS(unsigned long gps_listen_timeout) {
  //#ifdef SHOW_DEBUG
  Serial.print(F("Start: doGPS. timeout=")); Serial.println(gps_listen_timeout);Serial.print(F(" t=")); Serial.println(millis());
  //#endif
  
  bool hasFix = false;
  unsigned long gps_listen_startTime = millis(); 

  
  //now listen to gps till fix or time-out, once gps has a fix, the refresh should be ready within 2 data reads = less than 3 sec
  // gps read command:
  //#ifdef SHOW_DEBUGLEVEL2
  Serial.println(F("Listen to GPS stream:"));
  //#endif
  while((millis() - gps_listen_startTime) < (gps_listen_timeout * 1000L) && !hasFix) {
    // for NMEAgps
    while (gps.available(ss) && !hasFix) {
      hasFix = process_gps_datastream(gps.read()); 
    }
  }    
  //#ifdef SHOW_DEBUGLEVEL2
  Serial.println(F("Completed listening to GPS."));
  //#endif
  if(hasFix) {
    // movement detection
    if(l_lat_hist==0) l_lat_hist=l_lat;
    if(l_lon_hist==0) l_lon_hist=l_lon;
    l_lat_movement = abs(l_lat_hist - l_lat);
    l_lon_movement = abs(l_lon_hist - l_lon);
    l_lat_hist= (2*l_lat_hist + l_lat)/3; 
    l_lon_hist= (2*l_lon_hist + l_lon)/3; 
  }
}

//void put_TimeToFix_into_sendbuffer(int TimeToFix_Seconds) {  // time to fix onto gps coordinates
//  int TimeToFix_Calculate;  // this helps to calculate but no round-off yet
//  if ( TimeToFix_Seconds < 0) {
//    TimeToFix_Calculate=0;
//  } else if ( TimeToFix_Seconds <= (1 * 60) ) {  
//    TimeToFix_Calculate = TimeToFix_Seconds;                  // 0..60 sec  at 1 sec interval <==> values 0 .. 60 
//  } else if ( TimeToFix_Seconds <= (10 * 60) ) {    
//    TimeToFix_Calculate = 60 + (TimeToFix_Seconds - (1* 60) )/5 ;   // 1..10 min at 5 sec interval  <==> values 60 ..  168 
//  } else if ( TimeToFix_Seconds <= (60 * 60) ) {  
//    TimeToFix_Calculate = 168 + (TimeToFix_Seconds - (10 * 60) )/60 ;   // 10..60 min at 1 min interval <==> values 168 .. 218     
//  } else {
//    TimeToFix_Calculate = 218 + (TimeToFix_Seconds - (60 * 60) )/600 ;    // 1..7:00 hour at 10 min interval <==> values 218 ..254   
//  }
//  if (TimeToFix_Calculate>255) TimeToFix_Calculate = 255 ;                  //  more than 7 hour = 255
//  
//  uint8_t TimeToFix_bin = TimeToFix_Calculate;  // this can contain the values 0..255,
//      
//  myLoraWanData[11] = TimeToFix_bin;
//  #ifdef SHOW_DEBUG
//  Serial.print(F("TTF=")); Serial.print(TimeToFix_Seconds); Serial.print(F(" sec. bin=")); Serial.print(TimeToFix_bin);
//  #endif
//}

void gps_setStrings() {
  #ifdef SHOW_DEBUG
  Serial.print(F("   gps_setStrings. t=")); Serial.println(millis());
  #endif

  // Turning ON or OFF  GPS NMEA strings 
  // we need lat, lon, alt, HDOP, date  --> keep GGA, RMC

  // GLL = Lat/Lon time fix
  //ss.print(F("$PUBX,40,GLL,0,0,0,0*5C\r\n"));  // GLL OFF
  ss.print(F("$PUBX,40,GLL,1,1,1,0*5D\r\n"));  // GLL ON

  // ZDA = date, time
  ss.print(F("$PUBX,40,ZDA,0,0,0,0*44\r\n"));  // ZDA OFF
  //ss.print(F("$PUBX,40,ZDA,1,1,1,0*45\r\n"));  // ZDA ON 
  
  // VTG = Vector Track and speed over ground
  ss.print(F("$PUBX,40,VTG,0,0,0,0*5E\r\n"));  // VTG OFF
  //ss.print(F("$PUBX,40,VTG,1,1,1,0*5F\r\n"));  // VTG ON

  // GSV = Satellite in View. #sentences,sentence#,#sat,[sat PRN#, elev degr, azi degr, SNR,] *check
  ss.print(F("$PUBX,40,GSV,0,0,0,0*59\r\n"));  //GSV OFF
  //ss.print(F("$PUBX,40,GSV,1,1,1,0*58\r\n"));  //GSV ON

  // RMC = recommended minimum, no Alt
  // ss.print(F("$PUBX,40,RMC,0,0,0,0*47\r\n"));    // RMC OFF
  ss.print(F("$PUBX,40,RMC,1,1,1,0*46\r\n"));      // RMC ON 

  // GSA = Overall Satelite status. Auto/Manual,1/2/3 D fix, PRN1, ...PRN12 satt id, pdop,hdop,vdop,*check
  ss.print(F("$PUBX,40,GSA,0,0,0,0*4E\r\n"));  // GSA OFF
  //ss.print(F("$PUBX,40,GSA,1,1,1,0*4F\r\n"));  // GSA ON

  // GGA = Fix information. time,lat,N,lon,E,fix qual,num sat,hor dilution, alt,M,height geoid,M,time since DGPS,DGPS id, *check
  // ss.println(F("$PUBX,40,GGA,0,0,0,0*5A"));   // GGA OFF
  ss.println(F("$PUBX,40,GGA,1,1,1,0*5B"));   // GGA ON

  //GRS
  //$PUBX,40,GRS,0,0,0,0*5D // Turn OFF
  //$PUBX,40,GRS,1,1,1,0*5C // Turn ON

  //GST
  //$PUBX,40,GRS,0,0,0,0*5D // Turn OFF
  //$PUBX,40,GRS,1,1,1,0*5C // Turn ON

  //gps_read_chars(300);
  
//  unsigned long gps_listen_startTime = millis(); 
//  unsigned long gps_timeout = 3;  // sec
//  
//  int times_without_char=0;
//  while((millis() - gps_listen_startTime) < (gps_timeout * 1000L)) {
//    // for SHOW_DEBUGging
//    if (ss.available()) {
//      char c = ss.read();
//      Serial.write(c);
//    }
//    if(times_without_char++>30 && times_without_char<60) Serial.write(".");
//  }
//  Serial.println();
  
}


//-----------------------------------

void GPS_measure () {
  
  #ifdef SHOW_DEBUG
  Serial.print(F("Start: GPS_measure. t=")); Serial.println(millis());
  #endif
  
  // Clear gps buffer, or else it will re-tell old position if no fix is found. If no fix is found we want invalid location so TTNmapper does not get disturbed.
  l_lat = 0; l_lon = 0; l_alt = 678; hdopNumber = 99999;   // the zero position
    
  doGPS(3);
  // put gps values into send buffer
  put_gpsvalues_into_sendbuffer();
  #ifdef SHOW_DEBUGLEVEL2
  Serial.print(F("\Completed: GPS_measure. t=")); Serial.println(millis());
  #endif
}
void GPS_init() {  
  //#ifdef SHOW_DEBUG
  Serial.print(F("GPS init. t=")); Serial.println(millis());
  //#endif
    
  // load send buffer with dummy location 0,0. This location 0,0 is recognized as dummy by TTN Mapper and will be ignored
  l_lat = 0; l_lon = 0; l_alt = 678; hdopNumber = 99999;   // the zero position
  l_lat_hist = 0; l_lon_hist = 0; l_lat_movement = 0; l_lon_movement = 0; // movement detection
  put_gpsvalues_into_sendbuffer(); 
  
  //  Serial.print( F("The NeoGps people are proud to show their smallest possible size:\n") );
  //  Serial.print( F("NeoGps, fix object size = ") ); Serial.println( sizeof(gps.fix()) );
  //  Serial.print( F("NeoGps, NMEAGPS object size = ") ); Serial.println( sizeof(gps) );

  //Serial.flush();
  ss.begin(9600);
  
  // gps_requestColdStart();  // DO NOT USE: it seems this does a FACTORY RESET and delays getting a solid fix
  // gps_SetMode_gpsRfOn();  // could use this for enegry friendlyness, depends on gps device if getting new fix is efficient
  gps_setStrings();

  // if ballooning some GPS devices do not report location if in mode 6 and altitude above 5999 meter
  // gps_setNavMode(7); // 2=stationary, 3=pedestrian, 4=auto, 5=Sea, 6=airborne 1g, 7=air 2g, 8=air 4g -- with 6 no 2d fix supported
  //   gps_read_until_fix_or_timeout(30 * 60);  // after factory reset, time to first fix can be 15 minutes (or multiple).  gps needs to acquire full data which is sent out once every 15 minutes; sat data sent out once every 5 minutes
  //gps_read_chars(200);
  //gps_setPowerMode(2);
  
  // as part of init, do one measurement to load values into send buffer
  GPS_measure();
}
