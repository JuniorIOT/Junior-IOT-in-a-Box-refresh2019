# Junior-IOT-in-a-Box-refresh2019
Clean version started for use with ICTindeWolken IoT project

Related to project:
  https://github.com/JuniorIOT/Junior-IOT-in-a-box-Project

Credits:
  Ted v d T --> re-coding the Project

To use this project for a basic gps tracker:

(1) Assemble the device
  - begin with the Lora32u4
  - add curly antenna
  - add 400mAh Lipo to charger port
  - add BN-180 micro GPS by connecting GND-Tx-Rx-VCC to Gnd-10-11-3V3 on the Lora32u4.

(2) Test: load your Ardiuno environment
 - add the libraries from supplied zip
 - open a blink example and confirm correct workings of the device
 - open the supplied arduino_test_GPS.ino, and confirm correct workings of the device and gps. Leave the device connected so the GPS can get a fix on locaton.

(3) Register the device with TTN
 - In your TTN account, if needed create a TTN app with decoder as supplied in decoder.txt, and add HTTP integration to the S3 bucket (detailed instructions available). If ballooning, do not integrate with ttnmapper.
 - In your TTN app, add a device (have ttn generate all fields) and change to ABP mode and change to NOT check for frame counter. If ballooning, use bal as the first 3 letters in the device name.

(4) Program as GPS tracker
 - open supplied arduino_juniorIOT_Lora32u4_basic.ino
 - Confirm payload size is defined as: #define PAYLOADSIZE 40
 - Copy the devices' TTN sample code to define the keys to your keys.h
 - Send the program to the Lora32u4, and review progress in the serial monitor.

(5) Confirm and play
 - Check and confirm if seen in TTN console
 - Check and confirm if seen in S3 integration, at http://junioriotchallenge.nl/ttn_data/gamedata/nodes.json
 - Check and confirm if seen in game map http://junioriotchallenge.nl/game/
 - Check and confirm if seen in today's balloon tracker list http://junioriotchallenge.nl/ttn_data/gamedata/nodes_track.json and map http://junioriotchallenge.nl/track/ (only if device name starts with bal)

(6) Add tracker icons to the map?
 - create a 150x150 transparent png where image mainly is in the middle 100x100 pix. Ipload to S3 game /icons subfolder and optionally also to track/icons subfolder for balloons
