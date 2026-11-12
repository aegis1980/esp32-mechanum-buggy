#include <stdint.h>
#include "FastAccelStepper.h"
#include "MecanumBuggy.h"


// direction , step
uint8_t frontLeftPins[] = {32,13}; //x on cnc board
uint8_t frontRightPins[]= {33,16}; //y on cnc board
uint8_t rearLeftPins[]= {25,17}; // z on cnc board
uint8_t rearRightPins[] = {14,27}; // a oncnc board

 Wheel wheels[4];

// Arduino setup function. Runs in CPU 1
void setup() {
    //Serial.begin(115200);
   
    FastAccelStepperEngine engine =  FastAccelStepperEngine();
    engine.init();

    wheels[0] = {"FL", frontLeftPins[1], frontLeftPins[0], nullptr};
    wheels[1] = {"FR", frontRightPins[1], frontRightPins[0], nullptr};
    wheels[2] = {"RL", rearLeftPins[1], rearLeftPins[0], nullptr};
    wheels[3] = {"RR", rearRightPins[1], rearRightPins[0],  nullptr};

    for (uint8_t i = 0; i < 4; ++i) {
        wheels[i].stepper = engine.stepperConnectToPin(wheels[i].stepPin);
        if (wheels[i].stepper) {
            wheels[i].stepper->setDirectionPin(wheels[i].dirPin);
            wheels[i].stepper->setAcceleration(100);
            wheels[i].stepper->setSpeedInHz(500);      
            wheels[i].stepper->setEnablePin(26);
            wheels[i].stepper->setAutoEnable(true);

        } else {
            Serial.printf("Failed to init stepper %s\n", wheels[i].name);
        }
    }
}

// Arduino loop function
void loop() {
    
    for (uint8_t i = 0; i < 4; ++i) {
        wheels[i].stepper->moveTo(6400*4, true);
        delay(500);
        wheels[i].stepper->moveTo(0, true);
        delay(500);
    }

    
    // The main loop must have some kind of "yield to lower priority task" event.
    // Otherwise, the watchdog will get triggered.
    // If your main loop doesn't have one, just add a simple `vTaskDelay(1)`.
    // Detailed info here:
    // https://stackoverflow.com/questions/66278271/task-watchdog-got-triggered-the-tasks-did-not-reset-the-watchdog-in-time
    //vTaskDelay(1);

}
