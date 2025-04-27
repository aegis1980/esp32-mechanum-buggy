#include <stdint.h>



#include <Bluepad32.h>
#include "Mecanum.h"
#include "firmware.h"
//include "BatteryMonitor.h"
extern "C" {
    #include "esp_bt.h"
}


// Onboard LED pin (for ESP32-DevKit V1 WROOM)
const int LED_PIN = 2;

// from running dumpGmaepad()
const float LEFT_JOY[] = {-508.0f,4.0f,512.0f};
const float RIGHT_JOY[] = {-508.0f,4.0f,512.0f};
const float LEFT_TRIGGER = 1020.0f;
const float RIGHT_TRIGGER = 1020.0f;

const float MAX_STRAFE = 0.1f; //m/s
const float MAX_THROTTLE = 0.1f; //m/s
const float MAX_OMEGA = 0.5f; //rad/sec

// direction , step
uint8_t frontLeftPins[] = {32,13}; //x on cnc board
uint8_t frontRightPins[]= {33,16}; //y on cnc board
uint8_t rearLeftPins[]= {25,17}; // z on cnc board
uint8_t rearRightPins[] = {14,27}; // a oncnc board

Mecanum buggy(frontLeftPins,frontRightPins,rearLeftPins,rearRightPins);

//BatteryMonitor battery(34, 30000.0, 10000.0);  // ADC pin, R1, R2


ControllerPtr myControllers[BP32_MAX_GAMEPADS];

unsigned long buttonsHeldStartTime = 0;
bool buttonsWereHeld = false;

bool firmwareUpdate = false;



float mapFloat(float x, float in_min, float in_max, float out_min, float out_max, float in_centre = 0.0f) {
  if (in_min == in_max) {
    // Handle the case where the input range is zero to avoid division by zero.
    return (out_min + out_max) / 2.0f; // Return the midpoint of the output range.
  }

  if (x == in_centre) {
    return (out_min + out_max) / 2.0f; // Return the midpoint of the output range.
  }

  float result = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

  return result;
}

// This callback gets called any time a new gamepad is connected.
// Up to 4 gamepads can be connected at the same time.
void onConnectedController(ControllerPtr ctl) {
    bool foundEmptySlot = false;
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == nullptr) {
            Serial.printf("CALLBACK: Controller is connected, index=%d\n", i);
            // Additionally, you can get certain gamepad properties like:
            // Model, VID, PID, BTAddr, flags, etc.
            ControllerProperties properties = ctl->getProperties();
            Serial.printf("Controller model: %s, VID=0x%04x, PID=0x%04x\n", ctl->getModelName().c_str(), properties.vendor_id,
                           properties.product_id);
            myControllers[i] = ctl;
            foundEmptySlot = true;
            break;
        }
    }
    if (!foundEmptySlot) {
        Serial.println("CALLBACK: Controller connected, but could not found empty slot");
    }
}

void onDisconnectedController(ControllerPtr ctl) {
    bool foundController = false;

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == ctl) {
            Serial.printf("CALLBACK: Controller disconnected from index=%d\n", i);
            myControllers[i] = nullptr;
            foundController = true;
            break;
        }
    }

    if (!foundController) {
        Serial.println("CALLBACK: Controller disconnected, but not found in myControllers");
    }
}

void dumpGamepad(ControllerPtr ctl) {
    Serial.printf(
        "idx=%d, dpad: 0x%02x, buttons: 0x%04x, axis L: %4d, %4d, axis R: %4d, %4d, brake: %4d, throttle: %4d, "
        "misc: 0x%02x, gyro x:%6d y:%6d z:%6d, accel x:%6d y:%6d z:%6d\n",
        ctl->index(),        // Controller Index
        ctl->dpad(),         // D-pad
        ctl->buttons(),      // bitmask of pressed buttons
        ctl->axisX(),        // (-511 - 512) left X Axis
        ctl->axisY(),        // (-511 - 512) left Y axis
        ctl->axisRX(),       // (-511 - 512) right X axis
        ctl->axisRY(),       // (-511 - 512) right Y axis
        ctl->brake(),        // (0 - 1023): brake button
        ctl->throttle(),     // (0 - 1023): throttle (AKA gas) button
        ctl->miscButtons(),  // bitmask of pressed "misc" buttons
        ctl->gyroX(),        // Gyro X
        ctl->gyroY(),        // Gyro Y
        ctl->gyroZ(),        // Gyro Z
        ctl->accelX(),       // Accelerometer X
        ctl->accelY(),       // Accelerometer Y
        ctl->accelZ()        // Accelerometer Z
    );
}

void processGamepad(ControllerPtr ctl) {
    // There are different ways to query whether a button is pressed.
    // By query each button individually:
    //  a(), b(), x(), y(), l1(), etc...


    // holding share & options for 3 seconds puts buggy in firmware upgrade mode/
    bool shareHeld = ctl->miscStart();
    bool optionsHeld = ctl->miscSelect();

    if (shareHeld && optionsHeld) {
        if (buttonsHeldStartTime == 0) {
            // First moment both are detected held
            buttonsHeldStartTime = millis();
        } else {
            // Check how long they have been held
            if ((millis() - buttonsHeldStartTime >= 3000) && !buttonsWereHeld) {
                Serial.println("Share + Options held for 3 seconds!");
                
                ctl->playDualRumble(0 /* delayedStartMs */, 250 /* durationMs */, 0x80 /* weakMagnitude */,
                  0x40 /* strongMagnitude */);
                delay(350);
                ctl->playDualRumble(0 /* delayedStartMs */, 250 /* durationMs */, 0x80 /* weakMagnitude */,
                  0x40 /* strongMagnitude */);

                ctl->disconnect();
                delay(100);

                //disable bt on esp32 so does not screw up wifi comms
                btStop(); // Stop Bluetooth controller
                esp_bt_controller_disable(); // Disable controller
                delay(100);
                
                firmwareUpdate = true;
                enterFirmwareFlashMode();
                // So we don't trigger repeatedly
                buttonsWereHeld = true;
            }
        }
    } else {
        // Reset if any button is released
        buttonsHeldStartTime = 0;
        buttonsWereHeld = false;
    }


    if (ctl->a()) {

    }


    if (ctl->x()) {
        // Some gamepads like DS3, DS4, DualSense, Switch, Xbox One S, Stadia support rumble.
        // It is possible to set it by calling:
        // Some controllers have two motors: "strong motor", "weak motor".
        // It is possible to control them independently.
        //ctl->playDualRumble(0 /* delayedStartMs */, 250 /* durationMs */, 0x80 /* weakMagnitude */,
         //                   0x40 /* strongMagnitude */);
    }

    //stafe with left joy
    float strafe = mapFloat(
      static_cast<float>(ctl->axisX()), //left joy value
      LEFT_JOY[0],
      LEFT_JOY[2],
      -MAX_STRAFE,
      MAX_STRAFE,
      LEFT_JOY[1]
    );

    // spin with right joy
    float omega = mapFloat(
      static_cast<float>(ctl->axisRX()), //right joy value
      RIGHT_JOY[0],
      RIGHT_JOY[2],
      -MAX_OMEGA,
      MAX_OMEGA,
      RIGHT_JOY[1]
    );

    // throttle with right and left tirggers
    float b = static_cast<float>(ctl->brake());
    float t = static_cast<float>(ctl->throttle());

    float throttle = 0;
    if (b>0 && t>0){
      throttle = 0;
    } else {
      if (b>0){
        t = -b;
      }
      throttle =  mapFloat(
        t, //left joy value
        -LEFT_TRIGGER,
        RIGHT_TRIGGER,
        -MAX_THROTTLE,
        MAX_THROTTLE
      );
    }


    
  buggy.move(throttle,strafe,omega);
    // Another way to query controller data is by getting the buttons() function.
    // See how the different "dump*" functions dump the Controller info.
    //dumpGamepad(ctl);
}


void processControllers() {
    for (auto myController : myControllers) {
        if (myController && myController->isConnected() && myController->hasData()) {
            if (myController->isGamepad()) {
                processGamepad(myController);
            } else {
                Serial.println("Unsupported controller");
            }
        }
    }
}


// Arduino setup function. Runs in CPU 1
void setup() {
    Serial.begin(115200);
    Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
    const uint8_t* addr = BP32.localBdAddress();
    Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    // Setup the Bluepad32 callbacks
    BP32.setup(&onConnectedController, &onDisconnectedController);

    // "forgetBluetoothKeys()" should be called when the user performs
    // a "device factory reset", or similar.
    // Calling "forgetBluetoothKeys" in setup() just as an example.
    // Forgetting Bluetooth keys prevents "paired" gamepads to reconnect.
    // But it might also fix some connection / re-connection issues.
    //BP32.forgetBluetoothKeys();

    // Enables mouse / touchpad support for gamepads that support them.
    // When enabled, controllers like DualSense and DualShock4 generate two connected devices:
    // - First one: the gamepad
    // - Second one, which is a "virtual device", is a mouse.
    // By default, it is disabled.
    BP32.enableVirtualDevice(false);

    //battery.begin();
    //battery.onLowBattery(handleLowBattery);
}

// Arduino loop function. Runs in CPU 1.
void loop() {
    // This call fetches all the controllers' data.
    // Call this function in your main loop.
    if (!firmwareUpdate){
        bool dataUpdated = BP32.update();
        if (dataUpdated)
            processControllers();

        // The main loop must have some kind of "yield to lower priority task" event.
        // Otherwise, the watchdog will get triggered.
        // If your main loop doesn't have one, just add a simple `vTaskDelay(1)`.
        // Detailed info here:
        // https://stackoverflow.com/questions/66278271/task-watchdog-got-triggered-the-tasks-did-not-reset-the-watchdog-in-time

    vTaskDelay(1);
    }
   //delay(150);
}
