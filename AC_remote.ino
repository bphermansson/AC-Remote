/*
 * AC Remote
 * A code for turning of on and off an AC certain times. 
 * 
 * 
IR receiver connected to Arduino as in https://learn.sparkfun.com/tutorials/ir-communication
Sketch IRrecvDump gives a code when power button is pressed:
Could not decode message

Raw (200): -23616 4550 -4300 650 -1550 600 -500 600 -1600 600 -1550 650 -450 650 -450 650 -1550 650 -450 600 -500 650 -1550 650 -450 650 -450 650 -1550 600 -1600 600 -500 600 -1550 650 -450 650 -1550 650 -1550 650 -1550 600 -1600 600 -500 600 -1550 650 -1550 600 -1600 650 -450 650 -450 650 -450 650 -450 650 -1550 600 -500 650 -450 600 -1600 600 -1550 650 -1550 650 -450 650 -450 650 -450 650 -450 650 -450 600 -500 650 -450 650 -450 650 -1550 650 -1550 650 -1550 650 -1500 650 -1550 650 -5150 4500 -4300 650 -1500 700 -400 700 -1500 650 -1550 650 -450 650 -450 650 -1550 650 -450 600 -500 650 -1550 650 -450 650 -450 650 -1550 650 -1500 650 -450 650 -1550 650 -450 650 -1550 650 -1550 650 -1550 650 -1500 700 -400 700 -1500 650 -1550 650 -1550 650 -450 650 -450 650 -450 650 -450 650 -1550 650 -450 650 -450 600 -1550 650 -1550 650 -1550 650 -450 650 -450 650 -450 650 -450 650 -450 650 -450 650 -450 650 -450 650 -1550 650 -1500 700 -1500 650 -1550 650 -1550 600 

The first integer (200) is the number of values. The remote is a little odd. You first set the mode on the remote 
("Auto", "Cool", "Fan"). This doesn't transmitt anything. Then when you press power a code is sent
 to the AC, different codes for on/off, the code also differs depending on what mode is set. 
So make sure the remote is in the desired mode before reading codes. 

Remove the first value and minus signs (See http://forum.arduino.cc/index.php?topic=170429.0)
Add "," between values
Can be done by modifying the IRrecvdump code (remove the "if ((i % 2) == 1)" and change to "Serial.print(",")";
)
The "200" is the number of values and also the size of the array

Set times when to send on and of at line 107 
"if (hournow >= 10 && hournow <= 12) {" means the AC is on between 10.00AM and 12.59 AM. 

Pseudo code:
-Check hour of day
-If time is between 12pm and 9pm
-It's daytime, the AC shall be on
-Send IR on signal
else if time is after 8pm
-Send IR off signal
-Wait

Hardware:
-IR diode connected to pin 3 via 100 ohm.
-For reading code, a IR receiver connected to pin 11.
-DS1307 based RTC module on A4 and A5. 
-Indicator led at D4. 


*/


// IR library
#include <IRremote.h>
IRsend irsend;

// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"
RTC_DS1307 rtc;

boolean ACon=false;
int led = 4; 

// just added my own array for the raw signal
//unsigned int powerOn[148] = {4400,4250,550,1600,600,1550,600,1550,550,1600,550,500,550,550,550,1550,600,500,550,500,600,500,550,500,600,450,600,1550,600,1550,550,500,600,1550,550,550,550,500,600,500,550,500,550,500,600,500,550,1600,550,1550,600,1550,600,1550,600,1550,600,1550,550,1600,550,1550,600,500,550,500,600,500,550,500,550,550,550,500,600,450,600,500,550,500,550,1600,550,1600,550,500,600,500,550,1600,550,500,550,500,550,550,550,500,600,500,550,500,600,450,600,500,550,500,550,550,550,500,600,1550,600,450,600,500,550,500,550,550,550,500,600,450,600,500,550,500,600,1550,550,500,600,500,600,1550,550,500,600,500,550,500,550,500,600};
unsigned int powerOn[200] = {4500,4250,700,1500,650,450,650,1550,650,1550,650,450,650,450,650,1500,700,400,650,450,650,1550,650,450,650,450,650,1550,650,1500,700,400,700,1500,650,1550,650,450,650,1550,650,1550,600,1550,700,1500,650,1550,650,1550,600,500,650,1500,700,400,650,450,650,450,700,400,700,400,700,400,650,450,650,450,650,1550,650,450,650,450,600,500,600,450,650,450,650,1550,700,1500,650,450,650,1550,650,1550,600,1550,650,1550,650,1550,650,5100,4500,4300,650,1550,650,450,650,1500,700,1500,700,400,700,400,700,1500,650,450,650,450,650,1550,650,450,650,450,600,1550,700,1500,650,450,650,1550,650,1550,650,450,600,1550,700,1500,650,1550,650,1550,650,1500,700,1500,650,450,650,1550,650,450,650,450,650,450,650,450,650,450,650,450,600,500,600,500,600,1550,700,400,700,400,650,450,650,450,650,450,600,1600,650,1550,600,500,600,1550,650,1550,650,1550,650,1550,650,1500,650};
unsigned int powerOff[200] =  {4550,4250,700,1500,650,450,650,1550,650,1550,650,450,650,450,600,1550,650,450,650,450,650,1550,650,450,650,450,650,1550,650,1500,650,450,700,1500,650,450,700,1500,650,1550,650,1550,650,1500,650,450,650,1550,650,1550,650,1550,600,500,600,500,650,450,600,450,650,1550,650,450,650,450,650,1550,650,1550,650,1500,650,450,650,450,700,400,700,400,650,450,650,450,650,450,650,450,650,1550,650,1550,650,1500,650,1550,650,1550,650,5150,4450,4300,650,1550,650,450,650,1550,650,1500,700,400,650,450,650,1550,650,450,650,450,650,1550,650,450,650,450,650,1500,650,1550,650,450,650,1550,600,500,650,1550,650,1500,650,1550,650,1550,650,450,650,1500,700,1550,600,1550,650,450,650,450,650,450,650,450,650,1550,650,450,650,450,650,1550,600,1550,650,1550,650,450,650,450,650,450,650,450,650,450,600,500,650,450,650,450,650,1500,700,1500,650,1550,650,1550,650,1550,600};

void setup()
{
   pinMode(led, OUTPUT);
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
    //rtc.adjust(DateTime(2015, 7, 20, 14, 23, 0));
   }
   //startMillis = millis();
   // Set time at boot/upload
   rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

   Serial.println("Welcome to AC Remote");
   // Testing hardware by sending on and off code 
   Serial.println("Testing hardware by sending on and off code");
   Serial.println("On");
   digitalWrite(led, HIGH);
   irsend.sendRaw(powerOn,200,38);
   delay(5000);
   Serial.println("Off");
   digitalWrite(led, LOW);
   irsend.sendRaw(powerOff,200,38);

    // Show current time
    DateTime now = rtc.now();
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
   
   Serial.println("Done");
}

void loop() {     
  
      // Time to wait between runs
      unsigned int LONG_DELAY_MS = 60000;
      unsigned long startMillis;
      startMillis = millis();
      while (millis() - startMillis < LONG_DELAY_MS);
      Serial.print("Timeout - ");
      Serial.println(millis());
      // Find current hour 
      DateTime now = rtc.now();
      int hournow=now.hour();
      Serial.print(now.hour(), DEC);
      Serial.print(':');
      Serial.println(now.minute(), DEC);

      if (hournow >= 12 && hournow <= 20) { //From 12:00 to 20:59
       // It's daytime
       // Ac on or off?
       if (ACon == false) {
        //Off, turn it on
        irsend.sendRaw(powerOn,200,38); 
        digitalWrite(led, HIGH);
        Serial.print(now.hour(), DEC);
        Serial.print(':');
        Serial.println(now.minute(), DEC);
        Serial.println("AC on");
        ACon=true;

       }
       else {
        //AC is on, no nothing  
       }       
      }
      else {
        // It's night
        // AC on or off?
        if (ACon == true) {
          //On, turn it off
          irsend.sendRaw(powerOff,200,38); 
          digitalWrite(led, LOW);
          Serial.print(now.hour(), DEC);
          Serial.print(':');
          Serial.println(now.minute(), DEC);
          Serial.println("AC off");
          ACon=false;
        }
      }  
}
