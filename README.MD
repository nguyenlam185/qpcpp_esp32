# QP/C++ ESP32

Arduino library to use QP/C++ with the ESP32 and the Arduino SDK. 
Based on the [port](https://github.com/vChavezB/qpcpp/tree/esp32) developed for the ESP32.

## Requirements

- [Arduino ESP32 Core](https://github.com/espressif/arduino-esp32) >= v2.0.11

## How to test

Open any of the examples and upload to an ESP32. 

## Important notes

- Typically `QF::run()` should be executed as a last instruction before QP takes charge of executing the framework. As such, it should not return and instead give control to the scheduler of the host. However, the ESP32 framework (esp-idf) already runs the scheduler [before](https://github.com/espressif/arduino-esp32/blob/8ee5f0a11e5423a018e0f89146e05074466274db/cores/esp32/main.cpp#L55) `void setup()` and `void loop()`. This means that instead of running `QF::run()` in a loop, it suffices to execute it once as the last instruction in `void setup()`.

- No more QSpy on this port