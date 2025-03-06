
// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
#define RPM 120

// Since microstepping is set externally, make sure this matches the selected mode
// If it doesn't, the motor will move at a different RPM than chosen
// 1=full step, 2=half step etc.
#define MICROSTEPS 1

#define DIR 5
#define STEP 2
#define SLEEP 13 // optional (just delete SLEEP from everywhere if not used)

#include <Arduino.h>
#include "MecanumBuggy.h"

int frontLeftPins[] = {6,3};
int frontRightPins[]= {5,2};
int rearLeftPins[]= {7,4};
int rearRightPins[] = {13,12};

MecanumBuggy buggy(MOTOR_STEPS,frontLeftPins,frontRightPins,rearLeftPins,rearRightPins);


//Uncomment line to use enable/disable functionality
//BasicStepperDriver stepper(MOTOR_STEPS, DIR, S TEP, SLEEP);

void setup() {
    // if using enable/disable on ENABLE pin (active LOW) instead of SLEEP uncomment next line
    // stepper.setEnableActiveState(LOW);
}

void loop() {
  buggy.moveForward(200);
}


// #include <Arduino.h>

// int dp, dn;

// void setup() {
//   dn = 13;  //total no. of digital pins:

//   for (dp =0; dp <=  dn; dp++) 
//   //    \\/ :
//   // enter the pin number you want to start from  :
//   {
//     pinMode(dp,OUTPUT);
//   }
  
// }

// void loop() {
// for (dp =0; dp <=  dn; dp++) 
//   //    \\/ :
//   // enter the pin number you want to start from  :
//   {
//     digitalWrite(dp, HIGH);  // turn the LED on (HIGH is the voltage level) 
//     }
//     delay(1000);
//     for (dp =0; dp <=  dn; dp++) 
//   //    \\/ :
//   // enter the pin number you want to start from  :
//   {
//     digitalWrite(dp, LOW);   // turn the LED off by making the voltage LOW
//     }
//     delay(1000);  
// }
