#include <Wire.h>
#include <SPI.h>

#include <SD.h>

#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"

#include <SoftwareSerial.h>

//BMP
static constexpr int I2CSDA = 6; //
static constexpr int I2CSCL = 7; // change ports once set up

//SD CARD
const int TX_SD = 25;
const int RX_SD = 21;
const int CSn_SD = 22;
const int SCK_SD = 24;

#define seaLevelPressure (1013.25)
Adafruit_BMP3XX bmp;

SoftwareSerial XBee(12,11); // Define pins: (RX, TX)

File dataLog;

int teamID = 0;

int curTemp = 0;
int curPres = 0;
int curAlti = 0;
int prevAlti = 0;
int curVel = 0;
int prevVel = 0;
int Accel = 0;

int prevReadTime = 0;

void readSensors() {

  //Altimu
  if (! bmp.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }

  if(prevReadTime == 0){
    prevReadTime = millis()/1000;
  }

  curTemp = bmp.temperature;
  curPres = bmp.pressure / 100;

  prevAlti = curAlti;
  curAlti = bmp.readAltitude(seaLevelPressure);

  int timeDifference = (millis()/1000)-prevReadTime;

  int tempVel = curVel;
  curVel = (curAlti-prevAlti) / timeDifference;
  prevVel = tempVel;

  Accel = (curVel-prevVel) / timeDifference;
  

}

void displaySensors(){
  //Altimu
  Serial.print("Temperature = ");
  Serial.print(curTemp);
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(curPres);
  Serial.println(" hPa");

  Serial.print("Approx. Altitude = ");
  Serial.print(curAlti);
  Serial.println(" m");

  Serial.println();
  

}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  XBee.begin(9600);

  while (!Serial);

  if (!bmp.begin_I2C()) {
    Serial.println("Could not find a valid BMP3 sensor, check wiring!");
    while (1);
  }

  Wire.setSDA(I2CSDA);
  Wire.setSCL(I2CSCL);

  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);

  Serial.print("Initializing SD card");

  SPI.setRX(RX_SD);
  SPI.setTX(TX_SD);
  SPI.setSCK(SCK_SD);
  SD.begin(CSn_SD);

  dataLog = SD.open("test.csv", FILE_WRITE);
  //do stuff to the file
  dataLog.println("John,Jacob,Jingleheimer,Schmidt\n");
  dataLog.println("His,Name,Is,Mine");

  Serial.write(dataLog.read()); //print data to output for debug

  dataLog.close(); //REMEMBER TO CLOSE THE FILE

}

void loop() {
  // put your main code here, to run repeatedly:

}
