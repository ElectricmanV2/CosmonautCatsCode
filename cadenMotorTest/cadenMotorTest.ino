#include <AccelStepper.h>

AccelStepper motor(AccelStepper::DRIVER, 0, 1);

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

  motor.moveTo(90);
  motor.runSpeedToPosition();
  delay(2000);
  motor.setCurrentPosition(0);
  delay(2000);
}
