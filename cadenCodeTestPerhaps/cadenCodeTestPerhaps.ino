#include <Wire.h>
#include <SPI.h>
#include <LPS.h>

#include <SD.h>

#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"

#include <LSM6.h>

#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //http://librarymanager/All#SparkFun_u-blox_GNSS

static constexpr int I2CSDA = 4; //
static constexpr int I2CSCL = 5; // change ports once set up



//SD CARD
const int TX_SD = 25;
const int RX_SD = 21;
const int CSn_SD = 22;
const int SCK_SD = 24;

#define seaLevelPressure (1013.25)

LSM6 imu;
LPS ps;
File dataLog;
SFE_UBLOX_GNSS gps;

const int teamID = 0;

float curTemp = 0;
float curPres = 0;

float curAlti = 0;
float prevAlti = 0;

float curVel = 0;
float prevVel = 0;

long latitude = 0;
long longitude = 0;

float Accel[3] = {0,0,0};

int prevReadTime = 0;

void readSensors() {

  //Altimu
  imu.read();

  if(prevReadTime == 0){
    prevReadTime = millis()/1000;
  }

  //PressureSensor
  curTemp = ps.readTemperatureC();
  curPres = ps.readPressureMillibars()
  curAlti = ps.pressureToAltitudeMeters(curPres);

  latitude = gps.getLatitude();
  longitude = gps.getLongitude();

  /*
  curTemp = bmp.temperature;
  curPres = bmp.pressure / 100;

  prevAlti = curAlti;
  curAlti = bmp.readAltitude(seaLevelPressure);

  int timeDifference = (millis()/1000)-prevReadTime;

  int tempVel = curVel;
  curVel = (curAlti-prevAlti) / timeDifference;
  prevVel = tempVel;

  Accel = (curVel-prevVel) / timeDifference;
  */
  //Serial.println((imu.a.x * 0.061)/1000);
  Accel[0] = (imu.a.x * 0.061)/1000;
  Accel[1] = (imu.a.y * 0.061)/1000;
  Accel[2] = (imu.a.z * 0.061)/1000;
  serial.print(imu.readGyro());

  return;
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

  Serial.print("Accel X = ");
  Serial.print(Accel[0]);
  Serial.println(" m/s");

  Serial.print("Accel Y = ");
  Serial.print(Accel[1]);
  Serial.println(" m/s");

  Serial.print("Accel Z = ");
  Serial.print(Accel[2]);
  Serial.println(" m/s");

  Serial.println();
  
  return;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin();

  while (!Serial);

  Wire.setSDA(I2CSDA);
  Wire.setSCL(I2CSCL);

  if (!imu.init())
  {
    Serial.println("Failed to detect and initialize IMU!");
    while (1);
  }

  if (!ps.init())
  {
    Serial.println("Failed to autodetect pressure sensor!");
    while (1);
  }

  if (!myGNSS.begin()) //Connect to the u-blox module using Wire port
  {
    Serial.println(F("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing."));
    while (1);
  }

  myGNSS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
  myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR

  imu.enableDefault();
  ps.enableDefault();

  /*
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
  */

}

int count = 0;

void loop() {
  // put your main code here, to run repeatedly:
  
  count+=1;
  readSensors();
  if(count==4){
    displaySensors();
    count = 0;
  }
  delay(250);
}
