#include <Wire.h>
#include <SPI.h>
#include <LPS.h>

#include <SD.h>

#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"

#include <LSM6.h>

#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //http://librarymanager/All#SparkFun_u-blox_GNSS

#include <AccelStepper.h>

float const gConv = 0.10197162129779;

//Imu
static constexpr int I2CSDA = 4; //
static constexpr int I2CSCL = 5; // change ports once set up

// Volt Reader ACD0 , GP26, pin 31
const int voltReader = 26;

//xBee
const int xbWrite = 8; //pin 11 WRITE TO THE XBEE
const int xbRead = 9; //pin 12 READ FROM THE XBEE

//SD CARD
const int TX_SD = 19;
const int RX_SD = 16;
const int CSn_SD = 17;
const int SCK_SD = 18;

//Zoe
const int zoeRX = 13;
const int zoeTX = 12;

//LEDS
int rgbR = 20;
int rgbG = 21;
int rgbB = 22;

//Runcam
int mosfet = 2;

#define seaLevelPressure (1013.25)

LSM6 imu;
LPS ps;
File dataLog;
SFE_UBLOX_GNSS gps;

AccelStepper motor(AccelStepper::DRIVER, 11, 10);

const String teamID = "Cosmonaut Cats";

float curTemp = 0;
float curPres = 0;

float curAlti = 0;
float prevAlti = 0;

float curVel = 0;

long latitude = 0;
long longitude = 0;

float batVolt = 0;

// RGB CODES              R           G       B       RG       RB         GB
String states[6] = {"Launch Detect","Boost","Coast","Apogee","Descent","Landed"};
int flightState = 0;

float Accel[3] = {0,0,0};
float gs = 0.00;
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

  // Vel calc

  prevAlti = curAlti;

  int timeDifference = (millis()/1000)-prevReadTime;

  curVel = (curAlti-prevAlti) / timeDifference;
  prevReadTime = millis()/1000;

  //Serial.println((imu.a.x * 0.061)/1000);
  Accel[0] = (imu.a.x * 0.061)/1000;
  Accel[1] = (imu.a.y * 0.061)/1000;
  Accel[2] = (imu.a.z * 0.061)/1000;

  gs = ((Accel[0] + Accel[1] + Accel[2])/3)*gConv;


  gyro[0] = (imu.g.x * 0.061)/1000;
  gyro[1] = (imu.g.y * 0.061)/1000;
  gyro[2] = (imu.g.z * 0.061)/1000;

  batVolt = analogRead(A0) * (5.0 / 1023.0);

  teleData = teamID+","+curAlti+","+curTemp+","+curVel+","+Accel[0]+","+Accel[1]+","+Accel[2]+","+latitude+","+longitude+","+states[flightState]+","+batVolt+","+gyro[0]+","+gyro[1]+","+gyro[2];

  return;
}

void readGPS(){
  latitude = gps.getLatitude();
  longitude = gps.getLongitude();

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

void sendTelem(){
  Serial2.println(teleData);
  return;
}

void readTelem(){
  if (Serial2.available()){
    Serial.print("Received");
    String readString;

    readString = Serial2.readStringUntil('\n');
    readString = readString.substring(0,readString.length()-2);

    if(readString == "DEPLOY"){
      airbrakeAct(1);
      changeFlightState(2);
    }

    Serial.println("Received: " + readString);
    Serial2.println("Received: " + readString);
  }
  return;
}

void saveData(){
  
  dataLog = SD.open("test.csv", FILE_WRITE);
  //do stuff to the file
  if(dataLog){
    dataLog.println(teleData);
    dataLog.close();
  }
  else{
    Serial.println("No file? :(");
  }
  return;
}

void airbrakeAct(int input){
  if(input==1){ //Open airbrake
    motor.runToNewPosition(300);
  }
  else{ //Close airbrake
    motor.runToNewPosition(0);
  }
  return;
}

void changeFlightState(int n){
  flightState = n;
  //rgb
  switch(flightState){
    case 0: //Waiting for launch
      digitalWrite(rgbR, 1);
      digitalWrite(rgbG, 0);
      digitalWrite(rgbB, 0);
      break;
    case 1: //Boost
      digitalWrite(mosfet,1);
      digitalWrite(rgbR, 0);
      digitalWrite(rgbG, 1);
      digitalWrite(rgbB, 0);
      break;
    case 2: //Coast
      airbrakeAct(1);
      digitalWrite(rgbR, 0);
      digitalWrite(rgbG, 0);
      digitalWrite(rgbB, 1);
      break;
    case 3: //Apogee
      digitalWrite(rgbR, 1);
      digitalWrite(rgbG, 1);
      digitalWrite(rgbB, 0);
      break;
    case 4: //Descent
      airbrakeAct(0);
      digitalWrite(rgbR, 1);
      digitalWrite(rgbG, 0);
      digitalWrite(rgbB, 1);
      break;
    case 5: //Landed
      digitalWrite(mosfet,0);
      digitalWrite(rgbR, 0);
      digitalWrite(rgbG, 1);
      digitalWrite(rgbB, 1);
      break;
    default:
      digitalWrite(rgbR, HIGH);
      digitalWrite(rgbG, HIGH);
      digitalWrite(rgbB, HIGH);
      break;
  }
  return;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  Serial1.setTX(zoeTX);
  Serial1.setRX(zoeRX);
  Serial2.setTX(8);
  Serial2.setRX(9);

  Serial1.begin(9600); 

  Serial2.begin(9600);//xBee

  Wire.setSDA(I2CSDA);
  Wire.setSCL(I2CSCL);

  Wire.begin();

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

  if(gps.begin(Serial1) == true){
    Serial.println("Connected at 9600 baud!");
  }
  else{
    Serial.println(F("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing."));
    while (1);
  }


  gps.setSerialRate(9600);
  gps.setUART1Output(COM_TYPE_UBX);
  gps.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
  gps.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR

  imu.enableDefault();
  ps.enableDefault();

  
  //SD
  Serial.println("Initializing SD card");

  SPI.setRX(RX_SD);
  SPI.setTX(TX_SD);
  SPI.setSCK(SCK_SD);

  if(SD.begin(CSn_SD)){
    Serial.println("SD Initialized!");
  }
  else{
    Serial.println("SD did not init....");
  }

  if(SD.exists("test.csv")){
    SD.remove("test.csv");
  }

  pinMode(rgbR, OUTPUT);
  pinMode(rgbG, OUTPUT);
  pinMode(rgbB, OUTPUT);

  pinMode(mosfet, OUTPUT); //

  motor.setMaxSpeed(3000.0);
  motor.setAcceleration(4000.0);
  motor.setCurrentPosition(0);

  changeFlightState(0);
  Serial.println("FInished setup");
}

int count = 0;
long startTime;

void loop() {
  // put your main code here, to run repeatedly:
  startTime = millis();
  count+=1;
  
  readSensors();

  if(flightState==0 && gs > 10){
    //GO TO BOOST!!!
    startTime = millis();
    changeFlightState(1);
  }
  else if(flightState == 1 && abs(gs) <= 1 && ((millis()-startTime)/1000) > 4){
    //COASTING!!
    changeFlightState(2);
  }
  else if(flightState == 2 && abs(curVel) <= 20){
    //APOGEE!!
    changeFlightState(3);
  }
  else if(flightState == 3 && gs <= -1.5){
    //DESCENT!!
    changeFlightState(4);
  }
  else if(flightState==4 && abs(curVel) <= 5 && abs(gs) <= 1){
    //LANDED!!
    changeFlightState(5);
  }

  if(Serial2.available()){
    readTelem();
  }

  if(count%2==0){
    readGPS();
    //displaySensors();
    saveData();
    sendTelem();
  }
  if(count==4){
    count = 0;
  }

  while((millis()-startTime) < 250);
}
