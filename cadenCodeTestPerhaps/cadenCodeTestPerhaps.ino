#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"

static constexpr int I2CSDA = 6; //
static constexpr int I2CSCL = 7; // change ports once set up

#define seaLevelPressure (1013.25)
Adafruit_BMP3XX bmp;
int curTemp = 0;
int curPres = 0;
int curAlti = 0;

void readSensors() {

  //Altimu
  if (! bmp.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }

  curTemp = bmp.temperature;
  curPres = bmp.pressure / 100;
  curAlti = bmp.readAltitude(seaLevelPressure);
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
}

void loop() {
  // put your main code here, to run repeatedly:

}
