#ifndef MECANUM_H
#define MECANUM_H

#include <FastAccelStepper.h>


struct Wheel {
  const char* name;
  uint8_t stepPin;
  uint8_t dirPin;
  FastAccelStepper* stepper;
  float currentRpm;
};


class MecanumBuggy {

private:
  static constexpr uint8_t NUM_WHEELS = 4;
  static constexpr uint16_t STEPS_PER_REV = 200;
  static constexpr uint8_t MICROSTEPPING = 1;
  static constexpr uint32_t EFFECTIVE_STEPS_PER_REV = STEPS_PER_REV * MICROSTEPPING;
  static constexpr uint32_t ACCELERATION = 300;  // Steps/sec^2


  void _setWheelTargetRPM(Wheel& wheel, float rpm);
  void _calcSmallRpmChange();

  FastAccelStepperEngine _engine;

  Wheel _wheels[NUM_WHEELS];

  float _wheelbase;
  float _trackWidth;
  float _wheelRadius;
  float _smallRpmChange;

public:
    // Constructor: Initialize 4 stepper motors
    MecanumBuggy(uint8_t frontLeftPins[2], uint8_t frontRightPins[2], uint8_t rearLeftPins[2], uint8_t rearRightPins[2]);


    float maxStrafe=0;
    float maxThrottle=0;
    float maxOmega=0;

    // Movement functions

    unsigned int move(float throttle, float strafe, float omega);
    unsigned int stop(bool sudden);

    void setMaxStrafe(float s);// m/sec
    void setMaxThrottle(float t); //m/sec
    void setMaxOmega(float a); //degrees per sec




};

#endif
