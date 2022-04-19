#include <Arduino.h>
#include <sdios.h>

#include "logger.h"

#define PIN 13

#define SD_CS 5

#define SD_FAT_TYPE 3
#define SPI_SPEED SD_SCK_MHZ(4)

Vars vars;
ArduinoOutStream cout(Serial);

void setup() {
    Serial.begin(115200);
    pinMode(PIN, OUTPUT);
    //PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[13], PIN_FUNC_GPIO);

    //Serial.println("Setup Finished!");

}

void loop() {
    Vars vars;
    static Vars *vp = &vars;

    logger::fsm(vp, cout);

//    digitalWrite(PIN, HIGH);
//    delay(500);
//    digitalWrite(PIN, LOW);
//    delay(500);
}