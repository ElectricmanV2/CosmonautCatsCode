#include <AccelStepper.h>

// Define a stepper and the pins it will use
AccelStepper stepper(AccelStepper::DRIVER, 11, 10);

void setup()
{  
    stepper.setMaxSpeed(3000.0);
    stepper.setAcceleration(4000.0);
}

void loop()
{    
    stepper.runToNewPosition(0);
    stepper.runToNewPosition(300);
   
}
