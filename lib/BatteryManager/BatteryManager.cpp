#include <Arduino.h>

struct BmsStatus {
    float voltage;
    String onTime;
    bool relayOn;

    bool operator==(const BmsStatus& other) const {
        return voltage == other.voltage && onTime == other.onTime && relayOn == other.relayOn;
    }

    bool operator!=(const BmsStatus& other) const {
        return !(*this == other);
    }
};

struct Battery {
    float upperVoltage;
    float lowerVoltage;
    String name;
};

typedef void (*BatteryStatusCallback)(const BmsStatus& status);

class BatteryManager {
public:
    BatteryManager(int rxPin = 9, int txPin = 10, int uartNum = 1);

    void begin(unsigned long baudRate = 115200);
    void sendRawCommand(const String& command);
    bool available();
    void flushInput();

    void setOperatingMode(int mode);           // U-1 to U-6
    void setTiming(int duration);              // 0 to 999
    void setLowerVoltageLimit(float voltage);  // Dw02.9
    void setUpperVoltageLimit(float voltage);  // Up10.1
    void requestSettings();                    // Get
    void enableRelay();                        // On
    void disableRelay();                       // Off
    void startDataUpload();                    // Start
    void stopDataUpload();                     // Stop
    void sendCombinedCommands(const String& cmd1, const String& cmd2);

    void setStatusCallback(BatteryStatusCallback callback);
    void poll();

private:
    HardwareSerial* uart;
    int rxPin;
    int txPin;
    int uartNum;
    BatteryStatusCallback statusCallback = nullptr;
    String inputBuffer = "";
    BmsStatus lastStatus = { -1.0, "", false }; // start with invalid initial state

    BmsStatus parseStatusLine(const String& line);
};

BatteryManager::BatteryManager(int rxPin, int txPin, int uartNum) : rxPin(rxPin), txPin(txPin), uartNum(uartNum)
{
    uart = new HardwareSerial(uartNum);
}

void BatteryManager::begin(unsigned long baudRate)
{
    uart->begin(baudRate, SERIAL_8N1, rxPin, txPin);
}

void BatteryManager::sendRawCommand(const String& command)
{
    uart->println(command);
}

bool BatteryManager::available()
{
    return uart->available() > 0;
}

void BatteryManager::flushInput()
{
    while (uart->available()) {
        uart->read();
    }
    inputBuffer = "";
}

void BatteryManager::setOperatingMode(int mode)
{
    if (mode >= 1 && mode <= 6) {
        sendRawCommand("U-" + String(mode));
    }
}

void BatteryManager::setTiming(int duration)
{
    duration = constrain(duration, 0, 999);
    char buf[4];
    snprintf(buf, sizeof(buf), "%03d", duration);
    String cmd = String(buf);
    sendRawCommand(cmd);
}

void BatteryManager::setLowerVoltageLimit(float voltage)
{
    sendRawCommand("dw" + String(voltage, 1));
}

void BatteryManager::setUpperVoltageLimit(float voltage)
{
    sendRawCommand("up" + String(voltage, 1));
}

void BatteryManager::requestSettings()
{
    sendRawCommand("get");
}

void BatteryManager::enableRelay()
{
    sendRawCommand("on");
}

void BatteryManager::disableRelay()
{
    sendRawCommand("off");
}

void BatteryManager::startDataUpload()
{
    sendRawCommand("start");
}

void BatteryManager::stopDataUpload()
{
    sendRawCommand("stop");
}

void BatteryManager::sendCombinedCommands(const String& cmd1, const String& cmd2)
{
    sendRawCommand(cmd1 + "," + cmd2);
}

void BatteryManager::setStatusCallback(BatteryStatusCallback callback)
{
    statusCallback = callback;
}

BmsStatus BatteryManager::parseStatusLine(const String& rawline)
{
    BmsStatus status = {0.0, "", false};


    String cleaned = rawline.substring(rawline.indexOf(':') + 1);
    cleaned.replace("h", "");
    cleaned.replace(" ", "");


     // Now expect format: 11.8v,00:22:29,OP
    int firstComma = cleaned.indexOf(',');
    int secondComma = cleaned.indexOf(',', firstComma + 1);

    if (firstComma == -1 || secondComma == -1) {
        return status; // invalid format
    }

    String voltageStr = cleaned.substring(0, firstComma);
    String timeStr = cleaned.substring(firstComma + 1, secondComma);
    String stateStr = cleaned.substring(secondComma + 1);

    status.voltage = voltageStr.toFloat();
    status.onTime = timeStr;
    stateStr.trim();
    status.relayOn = stateStr.equals("OP");

    return status;
}

void BatteryManager::poll()
{
    while (uart->available()> 0 ) {
        char c = uart->read();
        if (c == '\n') {
            inputBuffer.trim();
            if (inputBuffer.length() > 0) {
                BmsStatus status = parseStatusLine(inputBuffer);
                if (status != lastStatus && statusCallback) {
                    lastStatus = status;
                    statusCallback(status);
                }
            }
            inputBuffer = "";
        } else {
            inputBuffer += c;
        }
    }
}
