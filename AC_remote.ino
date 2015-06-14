/*
IR receiver connected to Arduino as in https://learn.sparkfun.com/tutorials/ir-communication
Sketch IRrecvDump gives:
Could not decode message

Raw (200): -23616 4550 -4300 650 -1550 600 -500 600 -1600 600 -1550 650 -450 650 -450 650 -1550 650 -450 600 -500 650 -1550 650 -450 650 -450 650 -1550 600 -1600 600 -500 600 -1550 650 -450 650 -1550 650 -1550 650 -1550 600 -1600 600 -500 600 -1550 650 -1550 600 -1600 650 -450 650 -450 650 -450 650 -450 650 -1550 600 -500 650 -450 600 -1600 600 -1550 650 -1550 650 -450 650 -450 650 -450 650 -450 650 -450 600 -500 650 -450 650 -450 650 -1550 650 -1550 650 -1550 650 -1500 650 -1550 650 -5150 4500 -4300 650 -1500 700 -400 700 -1500 650 -1550 650 -450 650 -450 650 -1550 650 -450 600 -500 650 -1550 650 -450 650 -450 650 -1550 650 -1500 650 -450 650 -1550 650 -450 650 -1550 650 -1550 650 -1550 650 -1500 700 -400 700 -1500 650 -1550 650 -1550 650 -450 650 -450 650 -450 650 -450 650 -1550 650 -450 650 -450 600 -1550 650 -1550 650 -1550 650 -450 650 -450 650 -450 650 -450 650 -450 650 -450 650 -450 650 -450 650 -1550 650 -1500 700 -1500 650 -1550 650 -1550 600 

The first integer (200) is the number of values.

Remove the first value and minus signs (See http://forum.arduino.cc/index.php?topic=170429.0)
Add "," between values
Can be done by modifying the IRrecvdump code (remove the "if ((i % 2) == 1)" and change to "Serial.print(",")";
)
The "200" is the number of values and also the size of the array

Pseudo code:
-Check hour of day
-Check temperature in room
-If time is between 12pm and 8pm
-& if temp is > 23 degrees C
-It's hot, it's daytime, the AC shall be on
-Send IR on signal
else if time is after 8pm
-Send IR off signal
-Wait

*/

// Time to wait between runs
unsigned int LONG_DELAY_MS = 5000;
unsigned long startMillis;

// IR library
#include <IRremote.h>
IRsend irsend;

// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"
RTC_DS1307 rtc;

// just added my own array for the raw signal
//unsigned int powerOn[148] = {4400,4250,550,1600,600,1550,600,1550,550,1600,550,500,550,550,550,1550,600,500,550,500,600,500,550,500,600,450,600,1550,600,1550,550,500,600,1550,550,550,550,500,600,500,550,500,550,500,600,500,550,1600,550,1550,600,1550,600,1550,600,1550,600,1550,550,1600,550,1550,600,500,550,500,600,500,550,500,550,550,550,500,600,450,600,500,550,500,550,1600,550,1600,550,500,600,500,550,1600,550,500,550,500,550,550,550,500,600,500,550,500,600,450,600,500,550,500,550,550,550,500,600,1550,600,450,600,500,550,500,550,550,550,500,600,450,600,500,550,500,600,1550,550,500,600,500,600,1550,550,500,600,500,550,500,550,500,600};
unsigned int powerOn[200] = {4500,4300,600,1550,650,450,650,1550,650,1550,600,500,600,500,600,1550,650,450,650,450,650,1550,650,450,650,450,650,1550,600,1600,600,500,600,1550,650,450,650,1550,650,1550,600,1550,650,1550,650,450,650,1550,650,1550,600,1600,600,500,600,450,650,450,650,450,650,1550,650,450,650,450,650,1550,600,1550,650,1550,650,450,650,450,650,450,650,450,650,450,650,450,600,500,600,500,600,1600,600,1550,650,1550,650,1550,600,1600,600,5150,4500,4300,600,1550,650,450,650,1550,650,1550,650,450,650,450,600,1600,600,500,600,450,650,1550,650,450,650,450,650,1550,600,1600,600,500,600,1550,650,450,650,1550,650,1550,600,1600,600,1550,650,450,650,1550,650,1550,600,1600,600,500,600,500,600,450,650,450,650,1550,650,450,650,450,650,1550,600,1550,650,1550,650,450,650,450,650,450,650,450,650,450,650,450,650,450,650,450,600,1600,600,1550,650,1550,650,1550,600,1600,600};

void setup()
{
   Serial.begin(9600);
   // Start RTC
   #ifdef AVR
    Wire.begin();
   #else
    Wire1.begin(); // Shield I2C pins connect to alt I2C bus on Arduino Due
   #endif
   rtc.begin();
   if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
   }
   startMillis = millis();
   Serial.println("Welcome to AC Remote");
   
}

void loop() {

     // altered the code just to send/test my raw code
       irsend.sendRaw(powerOn,200,38);
       delay(5000);
       startMillis = millis();
       while (millis() - startMillis < LONG_DELAY_MS);
      Serial.print("Timeout - ");
      Serial.println(millis());
      // Find current hour
      DateTime now = rtc.now();
      int hournow=now.hour();
      Serial.print(hournow, DEC);

      
}
