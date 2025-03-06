#ifndef MECANUMBUGGY_H
#define MECANUMBUGGY_H

#include "BasicStepperDriver.h"
#include "MultiDriver.h"
#include "SyncDriver.h"

class MecanumBuggy {
public:
    // Constructor: Initialize 4 stepper motors
    MecanumBuggy(int steps, int frontLeftPins[2], int frontRightPins[2], int rearLeftPins[2], int rearRightPins[2]);

    // Movement functions
    unsigned int moveForward(int speed);
    long moveBackward(int speed);
    long strafeLeft(int speed);
    long strafeRight(int speed);
    long rotateClockwise(int speed);
    long rotateCounterClockwise(int speed);
    long stop();


private:

    BasicStepperDriver* _stepperLF;
    BasicStepperDriver* _stepperRF;
    BasicStepperDriver* _stepperLR;
    BasicStepperDriver* _stepperRR;

};

#endif
