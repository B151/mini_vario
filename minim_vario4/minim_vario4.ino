/*
Minimal vario
based on
http://taturno.com/code/VariometroV2.pde


*/

#include <Wire.h>
#include "SparkFun_MS5637_Arduino_Library.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
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
char buf [4]; // buffer for dtostrf
int buzzPin = 2;
int interval=50; //100
int samples=40;//40
int maxsamples=50;//50
float alt[51];
float tim[51];
float beep;
float Beep_period;
float varioold = 0;

float LastPercent = 0;

float newPercent = 0;

///

float startingPressure = 0.0;
unsigned long time = 0;

float toneFreq, toneFreqLowpass, pressure, lowpassFast, lowpassSlow ;

int ddsAcc;
float calcAltitude(float pressure){

  float A = pressure/1013.25;
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
   Beep_period=500;

  Wire.begin();
  
  // start & clear the display
  display.begin();
  display.clearDisplay();
  display.setTextSize(2);
    display.setTextColor(BLACK);
    display.setCursor(12,50);
    display.println("MINI Vario");
      display.setTextSize(2);
    display.println("");
       display.setTextSize(2);
    display.println("    0.2  ");
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
   display.setRotation(0);
   display.setTextSize(2);
  display.setTextColor(BLACK);
 //display.setFont (&FreeMonoBold12pt7b);
    
}

///////////////bargraph
void drawBar (float nPer){

  if(nPer < LastPercent){
   display.fillRect(1, 50 + (100-LastPercent), 10, LastPercent - nPer,  WHITE);     
  }
  else{
   display.fillRect(1, 50 + (100-nPer), 10, nPer - LastPercent,  BLACK);
  }    
  LastPercent = nPer;  
}
void loop ( ) 
{
   
// read MS5637
  float temperature = barometricSensor.getTemperature() ; 
  float pressure = barometricSensor.getPressure(); 
  float atm = pressure / 1013.25; // "standard atmosphere"
  float baltitude = calcAltitude(pressure); //caculation - in Meters 
  float tempo=millis();

 // Buffer the samples in the vectors fifo of height and time 
  for(int cc=1;cc<=maxsamples;cc++){
    alt[(cc-1)]=alt[cc];
    tim[(cc-1)]=tim[cc];
  }
  alt[maxsamples]=baltitude;
  tim[maxsamples]=tempo;
  
  // Linear interpolation (least squares) to determine the vertical variation rate 
  float stime=tim[maxsamples-samples];
  float N1=0;
  float N2=0;
  float N3=0;
  float D1=0;
  float D2=0;
  for(int cc=(maxsamples-samples);cc<maxsamples;cc++){
      N1+=(tim[cc]-stime)*alt[cc];
      N2+=(tim[cc]-stime);
      N3+=(alt[cc]);
      D1+=(tim[cc]-stime)*(tim[cc]-stime);
      D2+=(tim[cc]-stime);
  }
  float vario=0;
  vario=1000*((samples*N1)-N2*N3)/(samples*D1 - D2*D2);

    
//Serial.println(vario);
  //Serial.print(" ");
 
 
  // Output audio 
  if (tempo>50){     
    if ((tempo-beep)>Beep_period){
          beep=tempo;
         if (vario>0.05 && vario<0.99 ){
           Beep_period=280;
           tone(buzzPin,530+(500*vario) ,100);
                }
                 if (vario>1 && vario<10 ){
                 Beep_period=150;
                 tone(buzzPin,700+100*vario ,100);
                 }
                     
           if (vario<-3){
            Beep_period=200;
            tone(buzzPin,1500,200);
          }
    }
  } 

//Clear screen
  display.setCursor(5,25);
  display.fillRect(12, 0, 150, 180, WHITE);
     //print vario
    display.setCursor(18,35);
    display.setTextColor(BLACK,WHITE);
    display.setFont (&FreeMonoBold18pt7b);
    dtostrf (vario,5, 2, buf);
//Serial.println (variobuf);
    display.setTextSize(1);
    display.print (buf);
    display.setFont ();
    display.setTextSize(1);
    display.print ("m/s");
    //print altitude
      display.setCursor(15,80);
      display.setFont (&FreeMonoBold24pt7b);
      dtostrf (baltitude, 4,0, buf);
      display.print (buf);
      display.setFont ();
      display.setTextSize(2);display.print ("m");
      
   
    //print temperature
    display.setTextSize(3);
    //display.setFont (&FreeMonoBold12pt7b);
    display.setCursor(25,120);
    dtostrf (temperature, 5,2, buf);
    display.print (buf);display.setTextSize(1); display.print (char(247));display.print ("C");
    //print pressure
    display.setTextSize(2);
    display.setCursor(25,145);
    display.print (pressure);
    
// draw vario bar graph
  newPercent = int((vario/3)* 100.0);
 if (newPercent != LastPercent){
      drawBar(newPercent);        
    display.refresh();
}
}

/*
 * 
 * #include <pins_arduino.h>
 void loop() {

  analogReadResolution(10);
  analogReference(AR_INTERNAL1V0); //AR_DEFAULT: the default analog reference of 3.3V // AR_INTERNAL1V0: a built-in 1.0V reference
  
  // read the input on analog pin 0:
  int sensorValue = analogRead(ADC_BATTERY);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 4.3V):
  float voltage = sensorValue * (3.25 / 1023.0);
  // print out the value you read:
  Serial.print(voltage);
  Serial.println("V");
------------------------------------------------------------------------------
  float battery_voltage = 3.01; // use voltmeter

analogReadResolution(10);
  analogReference(AR_INTERNAL1V0); //AR_DEFAULT: the default analog reference of 3.3V // 
AR_INTERNAL1V0: a built-in 1.0V reference
  // read the input on analog pin 0:
  int sensorValue = analogRead(ADC_BATTERY);
  // Convert the analog reading (which goes from 0 - 1023) 
  float voltage = sensorValue * (battery_voltage / 1023.0);
  // print out the value you read:
  Serial.print("Voltage: ");
  Serial.print(voltage);
  Serial.println("V");

  float battery_percentage = ((voltage * 100) / battery_voltage);

  Serial.print("Batery Percentage: ");
  Serial.print(battery_percentage);
  Serial.println("%");

  analogReference(AR_DEFAULT);


}*/

