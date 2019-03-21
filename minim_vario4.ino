/*
Minimal vario
*/

#include <Wire.h>
#include "SparkFun_MS5637_Arduino_Library.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <avr/dtostrf.h>

// any pins can be used  - DO NOT use real SPI!
#define SHARP_SCK  4//9
#define SHARP_MOSI 5//8
#define SHARP_SS   7

// Set the size of the display here, e.g. 144x168!
Adafruit_SharpMem display(SHARP_SCK, SHARP_MOSI, SHARP_SS, 144, 168);
// The currently-available SHARP Memory Display (144x168 pixels)
// requires > 4K of microcontroller RAM; it WILL NOT WORK on Arduino Uno
// or other <4K "classic" devices!  The original display (96x96 pixels)
// does work there, but is no longer produced.

#define BLACK 0
#define WHITE 1

MS5637 barometricSensor;

/////
const unsigned char OSS = 0;  // Oversampling Setting
char variobuf [4];
// Calibration values
int ac1;
int ac2;
int ac3;
unsigned int ac4;
unsigned int ac5;
unsigned int ac6;
int b1;
int b2;
int mb;
int mc;
int md;

// b5 is calculated in bmp085GetTemperature(...), this variable is also used in bmp085GetPressure(...)
// so ...Temperature(...) must be called before ...Pressure(...).
long b5; 
int Conta=0;
int sec=0;

float m1=0;
float m2=0;
int buzzPin = 2;
int intervallo=100;
int campioni=40;
int maxcampioni=50;
float alt[51];
float tim[51];
float beep;
float periodoBeep;
float varioold = 0;



///

float startingPressure = 0.0;
unsigned long time = 0;

float toneFreq, toneFreqLowpass, pressure, lowpassFast, lowpassSlow ;

int ddsAcc;
float calcAltitude(float pressure){

  float A = pressure/101325;
  float B = 1/5.25588;
  float C = pow(A,B);
  C = 1 - C;
  C = C /0.0000225577;
  return C;
  }

void setup(void) {
  Serial.begin(9600);
   //while (!Serial);
  Serial.println("minimal Audio vario");
   periodoBeep=500;

  Wire.begin();
  
  // start & clear the display
  display.begin();
  display.clearDisplay();
  display.setTextSize(2);
    display.setTextColor(BLACK);
    display.setCursor(5,50);
    display.println("MINI Vario");
    display.refresh();
delay(5000);
display.clearDisplay();
  if (barometricSensor.begin() == false)
  {
    Serial.println("MS5637 sensor did not respond. Please check wiring.");
  }

  //Set the resolution of the sensor to the highest level of resolution: 0.016 mbar
barometricSensor.setResolution(ms5637_resolution_osr_8192);
  //barometricSensor.setResolution(ms5637_resolution_osr_4096);
  
 /** //Take 16 readings and average them
  startingPressure = 0.0;
  for (int x = 0 ; x < 16 ; x++)
    startingPressure += barometricSensor.getPressure();
  startingPressure /= (float)16;

  Serial.print("Starting pressure=");
  Serial.print(startingPressure);
  Serial.println("hPa");    

 pressure = barometricSensor.getPressure();
  lowpassFast = lowpassSlow = pressure;*/
   display.setRotation(3);
   display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setFont (&FreeMonoBold18pt7b);
    
}

void loop ( ) 
{
     display.setCursor(5,25);
     display.fillRect(0, 0, 168, 40, WHITE);
    //  display.setTextColor(WHITE,WHITE);
    //display.print(variobuf);
      //display.clearDisplay();
 //display.refresh();
  float media ; 
  //float temperature = bmp085GetTemperature ( bmp085ReadUT ( ) ) ; 
  float pressure = barometricSensor.getPressure(); 
  float atm = pressure / 101325; // "standard atmosphere"
  float baltitude = calcAltitude(pressure); //Uncompensated caculation - in Meters 
  float tempo=millis();

  // Buffero i campioni nei vettori fifo di altezza e tempo
  for(int cc=1;cc<=maxcampioni;cc++){
    alt[(cc-1)]=alt[cc];
    tim[(cc-1)]=tim[cc];
  };
  alt[maxcampioni]=baltitude;
  tim[maxcampioni]=tempo;
  
  // Effettuo l'interpolazione lineare (minimi quadrati) per determinare il 
  // tasso di varazione verticale
  float stime=tim[maxcampioni-campioni];
  float N1=0;
  float N2=0;
  float N3=0;
  float D1=0;
  float D2=0;
  for(int cc=(maxcampioni-campioni);cc<maxcampioni;cc++){
      N1+=(tim[cc]-stime)*alt[cc];
      N2+=(tim[cc]-stime);
      N3+=(alt[cc]);
      D1+=(tim[cc]-stime)*(tim[cc]-stime);
      D2+=(tim[cc]-stime);
  };
  float vario=0;
  vario=1000*((campioni*N1)-N2*N3)/(campioni*D1 - D2*D2);
/*Serial.print(0.1);
  Serial.print(" ");
  Serial.print(1);
  Serial.print(" ");*/


 
    
//Serial.println(vario);
  //Serial.print(" ");
  //display.refresh();
 
  // Output audio 
  if (tempo>5){      //Primi dieci secondi non beeppo
    if ((tempo-beep)>periodoBeep){
          beep=tempo;
         if (vario>0.05 && vario<10 ){
          
            periodoBeep=300;
            tone(buzzPin,830+(100*vario) ,100);
           
                     }
 /*if (vario>.5 && vario<.8 ){
            periodoBeep=200;
            tone(buzzPin,500+100*vario ,100);}*/
                     
           if (vario<-2.5){
            periodoBeep=300;
            tone(buzzPin,300,200);
          }
    }
  } 
      display.setCursor(5,35);
    display.setTextColor(BLACK,WHITE);
dtostrf (vario, 5, 2, variobuf);
Serial.println (variobuf);
    display.print (variobuf);
 varioold=vario;
    display.refresh();
}
 
