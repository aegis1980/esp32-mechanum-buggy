
#include "Mecanum.h"

#include <FastAccelStepper.h>
const double PI2 = PI * 2;

    // Constructor: Initialize 4 stepper motors
Mecanum::Mecanum(uint8_t frontLeftPins[2], uint8_t frontRightPins[2], uint8_t rearLeftPins[2], uint8_t rearRightPins[2]) : _wheelbase(0.28f), _trackWidth(0.28f), _wheelRadius(0.04f)
{

    _engine = FastAccelStepperEngine();
    _engine.init();
    
    _wheels[0] = {"FL", frontLeftPins[1], frontLeftPins[0], nullptr};
    _wheels[1] = {"FR", frontRightPins[1], frontRightPins[0], nullptr};
    _wheels[2] = {"RL", rearLeftPins[1], rearLeftPins[0], nullptr};
    _wheels[3] = {"RR", rearRightPins[1], rearRightPins[0],  nullptr};

    for (uint8_t i = 0; i < NUM_WHEELS; ++i) {
      _wheels[i].stepper = _engine.stepperConnectToPin(_wheels[i].stepPin);
      if (_wheels[i].stepper) {
        _wheels[i].stepper->setDirectionPin(_wheels[i].dirPin);
        _wheels[i].stepper->setAcceleration(ACCELERATION);
        _wheels[i].stepper->setEnablePin(26);
        _wheels[i].stepper->setAutoEnable(true);

      } else {
        Serial.printf("Failed to init stepper %s\n", _wheels[i].name);
      }
    }

}

void Mecanum::_setWheelRPM(Wheel& wheel, float rpm) {
  if (!wheel.stepper) return;

  if (rpm == 0.0f) {
    wheel.stepper->forceStop();
    return;
  }

  // Convert RPM → steps/sec
  float steps_per_sec = (abs(rpm) * EFFECTIVE_STEPS_PER_REV) / 60.0f;

  wheel.stepper->setSpeedInHz((uint32_t)steps_per_sec);

  if (rpm > 0){
    wheel.stepper->runForward();
  }else{
    wheel.stepper->runBackward();
  }
}

unsigned int Mecanum::move(float throttle, float strafe, float omega){


  // Calculate wheel linear velocity at contact point
  float v_fl = (strafe - throttle - (_wheelbase + _trackWidth) * omega) / 0.5f;
  float v_fr = (strafe + throttle + (_wheelbase + _trackWidth) * omega) / 0.5f;
  float v_rl = (-strafe + throttle - (_wheelbase + _trackWidth) * omega) / 0.5f;
  float v_rr = (-strafe - throttle + (_wheelbase + _trackWidth) * omega) / 0.5f;

  // Convert wheel speeds (m/s) to RPM
  float rpm_fl = (v_fl/ _wheelRadius) * ( 60 / PI2);
  float rpm_fr = -(v_fr/ _wheelRadius) * ( 60 / PI2);// negative
  float rpm_rl = (v_rl/ _wheelRadius) * ( 60 / PI2);
  float rpm_rr = -(v_rr/ _wheelRadius) * ( 60 / PI2);//naegtive
 


  _setWheelRPM(_wheels[0], rpm_fl);
  _setWheelRPM(_wheels[1], rpm_fr); 
  _setWheelRPM(_wheels[2], rpm_rl);
  _setWheelRPM(_wheels[3], rpm_rr);

  
  Serial.printf(
    "throttle: %.3f, strafe: %.3f omega: %.3f  ----  ",
    throttle,        
    strafe,        
    omega      
  );
  
  Serial.printf(
    "lf: %.3f, rf: %.3f lr: %.3f rr: %.3f\n",
      rpm_fl,    
      rpm_fr,    
      rpm_rl,
      rpm_rr     
   );

   return 1;
}

