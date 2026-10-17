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

// Volt Reader ACD0 , GP26, pin 31

//SD CARD
const int TX_SD = 25;
const int RX_SD = 21;
const int CSn_SD = 22;
const int SCK_SD = 24;

//LEDS
int rgbR = 0;
int rgbG = 0;
int rgbB = 0;

#define seaLevelPressure (1013.25)

LSM6 imu;
LPS ps;
File dataLog;
SFE_UBLOX_GNSS gps;

const String teamID = "Cosmonaut Cats";

float curTemp = 0;
float curPres = 0;

float curAlti = 0;
float prevAlti = 0;

float curVel = 0;
float prevVel = 0;

long latitude = 0;
long longitude = 0;

float batVolt = 0;

// RGB CODES              R           G       B       RG       RB         GB
String states[6] = {"Launch Detect","Boost","Coast","Apogee","Descent","Landed"};
int flightState = 0;

float Accel[3] = {0,0,0};
float gyro[3] = {0,0,0}; //roll, pitch, yaw

String teleData = ""; //TeamID , Altitude, Temperature, Velocity, Accel X, Accel Y, Accel Z, GPS Lat, GPS Long, Flight State, Bat Volt, Roll, Pitch, Yaw

int prevReadTime = 0;

void readSensors() {

  //Altimu
  imu.read();

  if(prevReadTime == 0){
    prevReadTime = millis()/1000;
  }

  //PressureSensor
  curTemp = ps.readTemperatureC();
  curPres = ps.readPressureMillibars();
  curAlti = ps.pressureToAltitudeMeters(curPres);

  latitude = gps.getLatitude();
  longitude = gps.getLongitude();

  // Vel calc

  prevAlti = curAlti;

  int timeDifference = (millis()/1000)-prevReadTime;

  int tempVel = curVel;
  curVel = (curAlti-prevAlti) / timeDifference;
  prevVel = tempVel;

  //Serial.println((imu.a.x * 0.061)/1000);
  Accel[0] = (imu.a.x * 0.061)/1000;
  Accel[1] = (imu.a.y * 0.061)/1000;
  Accel[2] = (imu.a.z * 0.061)/1000;

  gyro[0] = (imu.g.x);
  gyro[1] = (imu.g.y);
  gyro[2] = (imu.g.z);

  teleData = teamID+","+curAlti+","+curTemp+","+curVel+","+Accel[0]+","+Accel[1]+","+Accel[2]+","+latitude+","+longitude+","+states[flightState]+","+batVolt+","+gyro[0]+","+gyro[1]+","+gyro[2];

  return;
}

void displaySensors(){
  
  Serial.print("Team ID: ");
  Serial.println(teamID);

  Serial.print("Approx. Altitude = ");
  Serial.print(curAlti);
  Serial.println(" m");

  Serial.print("Temperature = ");
  Serial.print(curTemp);
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(curPres);
  Serial.println(" hPa");

  Serial.print("Velocity = ");
  Serial.print(curVel);
  Serial.println(" m/s");

  Serial.print("Accel X = ");
  Serial.print(Accel[0]);
  Serial.println(" m/s/s");

  Serial.print("Accel Y = ");
  Serial.print(Accel[1]);
  Serial.println(" m/s");

  Serial.print("Accel Z = ");
  Serial.print(Accel[2]);
  Serial.println(" m/s");

  Serial.print("GPS Latitude = ");
  Serial.println(latitude);

  Serial.print("GPS Longitude = ");
  Serial.println(latitude);

  Serial.print("Flight State: ");
  Serial.println(states[flightState]);

  Serial.print("Battery Voltage = ");
  Serial.println(batVolt);

  Serial.print("Roll = ");
  Serial.println(gyro[0]);

  Serial.print("Pitch = ");
  Serial.println(gyro[1]);

  Serial.print("Yaw = ");
  Serial.println(gyro[2]);

  Serial.println();
  
  return;
}

void changeFlightState(int n){
  flightState = n;
  //rgb
  switch(flightState){
    case 0:
      digitalWrite(rgbR, 1);
      digitalWrite(rgbG, 0);
      digitalWrite(rgbB, 0);
    case 1:
      digitalWrite(rgbR, 0);
      digitalWrite(rgbG, 1);
      digitalWrite(rgbB, 0);
    case 2:
      digitalWrite(rgbR, 0);
      digitalWrite(rgbG, 0);
      digitalWrite(rgbB, 1);
    case 3:
      digitalWrite(rgbR, 1);
      digitalWrite(rgbG, 1);
      digitalWrite(rgbB, 0);
    case 4:
      digitalWrite(rgbR, 1);
      digitalWrite(rgbG, 0);
      digitalWrite(rgbB, 1);
    case 5:
      digitalWrite(rgbR, 0);
      digitalWrite(rgbG, 1);
      digitalWrite(rgbB, 1);
    default:
      digitalWrite(rgbR, HIGH);
      digitalWrite(rgbG, HIGH);
      digitalWrite(rgbB, HIGH);
  }
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

  if (!gps.begin()) //Connect to the u-blox module using Wire port
  {
    Serial.println(F("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing."));
    while (1);
  }

  gps.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
  gps.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR

  imu.enableDefault();
  ps.enableDefault();

  //SD
  Serial.print("Initializing SD card");

  SPI.setRX(RX_SD);
  SPI.setTX(TX_SD);
  SPI.setSCK(SCK_SD);
  SD.begin(CSn_SD);

  dataLog = SD.open("test.csv", FILE_WRITE);
  /*
  //do stuff to the file
  dataLog.println("John,Jacob,Jingleheimer,Schmidt\n");
  dataLog.println("His,Name,Is,Mine");

  Serial.write(dataLog.read()); //print data to output for debug
  */
  dataLog.close(); //REMEMBER TO CLOSE THE FILE

  pinMode(rgbR, OUTPUT);
  pinMode(rgbG, OUTPUT);
  pinMode(rgbB, OUTPUT);

  changeFlightState(0);

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
