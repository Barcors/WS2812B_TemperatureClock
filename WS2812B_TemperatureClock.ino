/*
2017-12-01 working:
-time,
-time adjust,
-autobrightness,
-day light saving time,
-DS18B20 from Dallas,
-positive temperature,
-negative temperature,
-minus in front of clock.
*/
///////////////////////////// Used libraries
#include <DS3232RTC.h>
#include <RTClib.h> //
#include <Time.h> 
#include <TimeLib.h>
#include <FastLED.h> //
#include <OneWire.h> 
#include <DallasTemperature.h>

RTC_DS3231 rtc;

////////////////////////////////////////////Defining pins on Arduino nano pro
#define NUM_LEDS 118 // 4+((7*4)*4)+2  Number of LEDs in the project
#define COLOR_ORDER BRG  // Define color order for your strip
#define DATA_PIN 6  // Data pin for led comunication
#define DST_PIN 2  // Define DST adjust button pin
#define MIN_PIN 4  // Define Minutes adjust button pin
#define HUR_PIN 5  // Define Hours adjust button pin
#define BRI_PIN 3  // Define Light sensor pin
#define ONE_WIRE_BUS 9 // Data wire of Dalas sensor is plugged into pin D3 on the Arduino 

// Setup a oneWire instance to communicate with any OneWire devices  
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

////////////// Constructing digits and symbols
CRGB leds[NUM_LEDS]; // Define LEDs strip
                    // 0,0,0,0
                    // 1,1,1,1
                    //  1 2 3 4 5 6 7 8 9 10111213141516171819202122232425262728    one digit led number
byte digits[12][28] = {{0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  // Digit 0
                       {0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1},   // Digit 1
                       {1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},   // Digit 2
                       {1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1},   // Digit 3
                       {1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1},   // Digit 4
                       {1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1},   // Digit 5
                       {1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},   // Digit 6
                       {0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1},   // Digit 7
                       {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},   // Digit 8
                       {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1},   // Digit 9 | 2D Array for numbers on 7 segment
                       {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0},   // Digit *0
                       {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0}};  // Digit C
                       
bool Dot = true;  //Dot state
bool DST = false; //DST state
bool TempShow = false;
int last_digit = 0;

// int ledColor = 0x0000FF; // Color used (in hex)
long ledColor = CRGB::DarkOrchid; // Color used (in hex)
//long ledColor = CRGB::MediumVioletRed;
//Random colors i picked up
long ColorTable[16] = {
  CRGB::Amethyst,
  CRGB::Aqua,
  CRGB::Blue,
  CRGB::Chartreuse,
  CRGB::DarkGreen,
  CRGB::DarkMagenta,
  CRGB::DarkOrange,
  CRGB::DeepPink,
  CRGB::Fuchsia,
  CRGB::Gold,
  CRGB::GreenYellow,
  CRGB::LightCoral,
  CRGB::Tomato,
  CRGB::Salmon,
  CRGB::Red,
  CRGB::Orchid
};


void setup(){ 
  Serial.begin(9600); 
  LEDS.addLeds<WS2812B, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS); // Set LED strip type
  LEDS.setBrightness(75); // Set initial brightness
  pinMode(DST_PIN, INPUT_PULLUP); // Define DST adjust button pin
  pinMode(MIN_PIN, INPUT_PULLUP); // Define Minutes adjust button pin
  pinMode(HUR_PIN, INPUT_PULLUP); // Define Hours adjust button pin
  TempShow = false; // do not show temperature
  sensors.begin();
  sensors.requestTemperatures(); // Send the command to get temperature readings
   
} 

// Get time in a single number, if hours will be a single digit then time will be displayed 155 instead of 0155////////////
int GetTime(){
  tmElements_t Now;
  RTC.read(Now);
  int hour=Now.Hour;
  int minutes=Now.Minute;
  int second =Now.Second;
  if (second % 2==0) {Dot = false;}
    else {Dot = true;};
  return (hour*100+minutes);
  };

//////////////////////////////// Check Light sensor and set brightness accordingly///////////////////////////////////
void BrightnessCheck(){
  const byte sensorPin = BRI_PIN; // light sensor pin
  // const byte brightnessLow = 100; // Low brightness value
  //const byte brightnessHigh = 170; // High brightness value
  int sensorValue = analogRead(sensorPin); // Read sensor
  Serial.print("Ambient is: ");
  Serial.println(sensorValue);
  sensorValue = map(sensorValue, 0, 255, 10, 100);
  LEDS.setBrightness(sensorValue);
  // if (sensorValue <= 10) {LEDS.setBrightness(brightnessHigh);}
  // else {LEDS.setBrightness(brightnessLow);}  
  };
  
//////////////////////////////////// Convert time to array needed for WS2812B /////////////////////
void TimeToArray(){
  int Now = GetTime();  // Get time
  
  int cursor = 118; // last led number
    leds[0]=0x000000; // blanking dash
    leds[1]=0x000000; // blanking dash
    leds[2]=0x000000; // blanking dash
    leds[3]=0x000000; // blanking dash
    
   Serial.print("Time is: ");Serial.println(Now);
  if (DST){   // if DST is true then add one hour
   Now+=100;
   Serial.print("DST is ON, time set to : ");Serial.println(Now);
  }; 
  if (Dot){leds[60]=ledColor;
           leds[61]=ledColor;}   
    else  {leds[60]=0x000000;
           leds[61]=0x000000;
    };
    
  for(int i=1;i<=4;i++){
    int digit = Now % 10; // get last digit in time
   //////////////////////////// 4 digit control: 
    if (i==1){
      Serial.print("Digit 4 is : ");
      Serial.print(digit);
      Serial.print(" ");
      cursor = 90;
    for(int k=0; k<=27;k++){ 
         Serial.print(digits[digit][k]);
        if (digits[digit][k]== 1){leds[cursor]=ledColor;}
         else if (digits[digit][k]==0){leds[cursor]=0x000000;};
         cursor ++;
        };
       Serial.println();

      if (digit != last_digit)
      {
        cylon();
        ledColor =  ColorTable[random(16)];
      }
      last_digit = digit;
      }
  //////////////////////////// 3 digit control:
    else if (i==2){
       Serial.print("Digit 3 is : ");
       Serial.print(digit);
       Serial.print(" ");
      cursor =62;
    for(int k=0; k<=27;k++){ 
         Serial.print(digits[digit][k]);
        if (digits[digit][k]== 1){leds[cursor]=ledColor;}
         else if (digits[digit][k]==0){leds[cursor]=0x000000;};
         cursor ++;
        };
       Serial.println();
      }
//////////////////////////// 2 digit control: 
    else if (i==3){
       Serial.print("Digit 2 is : ");
       Serial.print(digit);
       Serial.print(" ");
      cursor =32;
      for(int k=0; k<=27;k++){ 
         Serial.print(digits[digit][k]);
        if (digits[digit][k]== 1){leds[cursor]=ledColor;}
         else if (digits[digit][k]==0){leds[cursor]=0x000000;};
         cursor ++;
        };
       Serial.println();
      }
  //////////////////////////// 1 digit control:
    else if (i==4){
       Serial.print("Digit 1 is : ");
       Serial.print(digit);
       Serial.print(" ");
      cursor =4;
      for(int k=0; k<=27;k++){ 
         Serial.print(digits[digit][k]);
        if (digits[digit][k]== 1){leds[cursor]=ledColor;}
         else if (digits[digit][k]==0){leds[cursor]=0x000000;};
         cursor ++;
        };
       Serial.println();
      }
    Now /= 10;
  }; 
};
//////////////////////////////////////////////////// End of time show ///////////////////////////////////////////////////////
////////////////////////////////////////////// Convert temperature to array needed for WS2812B display ////////////////////
void TempToArray(){
  tmElements_t tm;
  RTC.read(tm);  // read time
  if (tm.Second != 10) {        //if seconds are les 20
  TempShow = false;           // then do not show temperature
    return;                     // and still show time
  }
 TempShow = true;              //but if so - show temperature
  sensors.requestTemperatures(); // Send the command to get temperature readings
  int celsius = sensors.getTempCByIndex(0);
  Serial.print("Temp is: ");
  Serial.println(celsius);
  
  int cursor = 118; // what is last led number
    
    leds[60]=0x000000;
    leds[61]=0x000000;
  int digit = celsius; 
///////////////////////////////////////////////////////////// all digits controll:
  for(int i=1;i<=5;i++){
//////////////////////////// 4 digit control:
   if (i==1){
      Serial.print("Digit 4 is : ");
      Serial.print(digit);
      Serial.print(" ");
   cursor = 90;  // 4 digit begins
      for(int k=0; k<=27;k++){ 
        Serial.print(digits[11][k]);
        if (digits[11][k]== 1){leds[cursor]=ledColor;}
         else if (digits[11][k]==0){leds[cursor]=0x000000;};
         cursor ++;
        };
      Serial.println();
    }
//////////////////////////// 3 digit control:
    else if (i==2){
      Serial.print("Digit 3 is : ");
      Serial.print(digit);
      Serial.print(" ");
   cursor =62;  // 3 digit begins
      for(int k=0; k<=27;k++){ 
        Serial.print(digits[10][k]);
        if (digits[10][k]== 1){leds[cursor]=ledColor;}
        else if (digits[10][k]==0){leds[cursor]=0x000000;};
         cursor ++;
        };
      Serial.println();
      }
 //////////////////////////// 2 digit control:  
    else if (i==3){
      int digit = abs(celsius % 10);  // taking only module (only temperature value without minus sign)
      Serial.print("Digit 2 is : ");
      Serial.print(digit);
      Serial.print(" ");
      
   cursor =32;  // 2 digit begins
      
      for(int k=0; k<=27;k++){ 
        Serial.print(digits[digit][k]);
        if (digits[digit][k]== 1){leds[cursor]=ledColor;}
         else if (digits[digit][k]==0){leds[cursor]=0x000000;};
         cursor ++;
        };
      Serial.println();
      }
 //////////////////////////// 1 digit control:
    else if (i==4){
      int digit = abs(celsius / 10);  // taking only module (only temperature value without minus sign)
      Serial.print("Digit 1 is : ");
      Serial.print(digit);
      Serial.print(" ");
   cursor =4;  // 1 digit begins
       for(int k=0; k<=27;k++){ 
        Serial.print(digits[digit][k]);
              if (digits[digit][k]==1){leds[cursor]=ledColor;}
         else if (digits[digit][k]==0){leds[cursor]=0x000000;};
         cursor ++;
        };
      Serial.println();
      }
//////////////////////////// dash control for negative temperature ////////////////////////
    else if (i==5){
      int digit = celsius % 10;
      Serial.print("Digit 1 is : ");
      Serial.print(digit);
      Serial.print(" ");
   cursor =0;  // dash begins
       for(int k=0; k<=3;k++){ 
        Serial.print("dot ");
        if (celsius < 0){leds[cursor]=ledColor;}
         else if (celsius >= 0){leds[cursor]=0x000000;};
         cursor ++;
        };
      Serial.println();
      }
   }; 
};

//////////////////////////////////////////////////////////////////// Tempshow End //////////////////////////

//////////////////////////////////////////////////////////////// Daylight saving time ////////////////////
void DSTcheck(){
   int buttonDST = digitalRead(2);
   // Serial.print("DST is: ");Serial.println(DST);
   if (buttonDST == LOW){
    if (DST){
      DST=false;
      // Serial.print("Switching DST to: ");Serial.println(DST);
      }
      else if (!DST){
        DST=true;
        // Serial.print("Switching DST to: ");Serial.println(DST);
      };
   delay(500);   
   };
  }
//////////////////////////////////////////////////////// End of daylight saving time function ////////////////////////

/////////////////////////////////////////////////////////////////// Time adjust function /////////////////////////////////
void TimeAdjust(){
  int buttonH = digitalRead(HUR_PIN);
  int buttonM = digitalRead(MIN_PIN);
  if (buttonH == LOW || buttonM == LOW){
    delay(500);
    tmElements_t Now;
    RTC.read(Now);
    int hour=Now.Hour;
    int minutes=Now.Minute;
    int second =Now.Second;
      if (buttonH == LOW){
        if (Now.Hour== 23){Now.Hour=0;}
          else {Now.Hour += 1;};
        }else {
          if (Now.Minute== 59){Now.Minute=0;}
          else {Now.Minute += 1;};
          };
    RTC.write(Now); 
    }
  }
////////////////////////////////////////////////////////////////// End of time adjust function /////////////////////////////

/////////////////////////////////////////////////////////////////////// Coool effect function ///////////////////////////////////
void fadeall() { for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(250); }}
void cylon () {
  static uint8_t hue = 0;
    Serial.print("x");
  // First slide the led in one direction
  for(int i = 0; i < NUM_LEDS; i++) {
    // Set the i'th led to red 
    leds[i] = CHSV(hue++, 255, 255);
    
    // Show the leds
    FastLED.show(); 
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    fadeall();
    // Wait a little bit before we loop around and do it again
    delay(15);
  }
  Serial.print("x");

  // Now go in the other direction.  
  for(int i = (NUM_LEDS)-1; i >= 0; i--) {
    // Set the i'th led to red 
    leds[i] = CHSV(hue++, 255, 255);
    // Show the leds
    FastLED.show();
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    fadeall();
    // Wait a little bit before we loop around and do it again
    delay(15);
  }
}
///////////////////////////////////////////////// Main program loop ///////////////////////////////////////////////////  
void loop()
{ 
  BrightnessCheck(); // Check brightness
  DSTcheck(); // Check DST
  TimeAdjust(); // Check to se if time is geting modified
  TimeToArray(); // Show time
  TempToArray(); // Show temperature
  FastLED.show(); // Display leds array
  if (TempShow == true) delay (5000);
}
/////////////////////////////////////////////////// End of program ////////////////////////////////////////////////
