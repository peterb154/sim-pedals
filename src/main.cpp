#include <Arduino.h>
#include <HX711.h>
#include <Joystick.h>
#include <ADS1X15.h>

//#define DEBUG 1

//#define calibration_factor -67050 // Brake Load cell - lbs
//#define calibration_factor -7050 // Brake Load cell - default
#define calibration_factor -1024 // Lower value seems to increase sensitivity
#define JS_MIN 0
#define JS_MAX 4096

#define DOUT  5
#define CLK  4

HX711 scale;
ADS1115 ADS(0x48);

Joystick_ Joystick(
        JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD,
        0, 0,                 // Button Count, Hat Switch Count
        false, false, false,  // X, Y, Z
        true, true, true,     // Rx (potentiometer), Ry (Brake load cell), Rz (potentiometers)
        false, false,         // Rudder, throttle
        false, false, false// Accelerator, brake, steering
);

int16_t Brake = 0;
int16_t lastBrakeValue = 0;
int16_t lastThrottleValue = 0;
int16_t lastClutchValue = 0;

void setup() {
#ifdef DEBUG
    Serial.begin(9600);
#endif
    Joystick.setRxAxisRange(JS_MIN, JS_MAX);
    Joystick.setRyAxisRange(JS_MIN, JS_MAX);
    Joystick.setRzAxisRange(JS_MIN, JS_MAX);
    ADS.begin();
    Joystick.begin();
    scale.begin(DOUT, CLK);
    scale.tare();
    scale.set_scale(calibration_factor);
}

void loop() {

    ADS.setGain(0);

    int16_t Throttle = ADS.readADC(0);
    Throttle = map(Throttle, 11100, 25000, JS_MIN, JS_MAX);
    if (Throttle < 0) {
        Throttle = 0;
    }
    if (lastThrottleValue != Throttle) {
        Joystick.setRxAxis(Throttle);
        lastThrottleValue = Throttle;
    }


    int16_t Clutch = ADS.readADC(1);
    Clutch = map(Clutch, 13190, 10500, JS_MIN, JS_MAX);
    if (Clutch < 0) {
        Clutch = 0;
    }
    if (lastClutchValue != Clutch) {
        Joystick.setRzAxis(Clutch);
        lastClutchValue = Clutch;
    }


    Brake = -scale.get_units(); // flip yellow and green wire
    Brake = map(Brake, 0, 1024, JS_MIN, JS_MAX);
    // Brake can fall below zero due to board flexing
     if (Brake < 0) {
         Brake = 0;
     }
    if (lastBrakeValue != Brake) {
        Joystick.setRyAxis(Brake);
        lastBrakeValue = Brake;
    }
#ifdef DEBUG
    Serial.print("Throttle: ");
    Serial.print(Throttle);
    Serial.print(", Clutch: ");
    Serial.print(Clutch);
    Serial.print(", Brake: ");
    Serial.println(Brake);
    delay(200);
#endif
}
