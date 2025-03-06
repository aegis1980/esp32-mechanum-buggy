#include "MecanumBuggy.h"

#include "BasicStepperDriver.h"
#include "MultiDriver.h"
#include "SyncDriver.h"


    // Constructor: Initialize 4 stepper motors
MecanumBuggy::MecanumBuggy(int steps, int frontLeftPins[2], int frontRightPins[2], int rearLeftPins[2], int rearRightPins[2])
{
    _stepperLF = new BasicStepperDriver(steps,frontLeftPins[0],frontLeftPins[1]);
    _stepperRF = new BasicStepperDriver(steps,frontRightPins[0],frontRightPins[1]);
    _stepperLR = new BasicStepperDriver(steps,rearLeftPins[0],rearLeftPins[1]);
    _stepperRR = new BasicStepperDriver(steps,rearRightPins[0],rearRightPins[1]);

    //_stepperLF->setEnableActiveState(HIGH);
    //_stepperRF->setEnableActiveState(HIGH);
    //_stepperLR->setEnableActiveState(HIGH);
    //_stepperRR->setEnableActiveState(HIGH);
}

    // Movement functions
unsigned int MecanumBuggy::moveForward(int speed){
    _stepperLF->begin(speed);
    _stepperRF->begin(speed);
    _stepperLR->begin(speed);
    _stepperRR->begin(speed);

    _stepperLF->startMove(10000000); 
    _stepperRF->startMove(10000000); 
    _stepperLR->startMove(10000000); 
    _stepperRR->startMove(10000000); 

    unsigned wait_time_micros = _stepperLF->nextAction();
    _stepperRF->nextAction();
    _stepperLR->nextAction();
    _stepperRR->nextAction();

   return wait_time_micros;
}


    long moveBackward(int speed);
    long strafeLeft(int speed);
    long strafeRight(int speed);
    long rotateClockwise(int speed);
    long rotateCounterClockwise(int speed);
    long stop();
