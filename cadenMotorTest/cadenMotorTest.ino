#include <AccelStepper.h>

AccelStepper motor(AccelStepper::DRIVER, 11, 10);

void setup() {
  // put your setup code here, to run once:
  motor.setSpeed(300.0);
  motor.setAcceleration(100.0);
  motor.setCurrentPosition(0);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.begin(9600);
  Serial.println("Stepping");

  motor.moveTo(5000);
  delay(2000);
  motor.setCurrentPosition(0);
  delay(2000);
}
