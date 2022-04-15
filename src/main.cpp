#include <Arduino.h>
#include <sdios.h>

#include "logger.h"
#include "dashboard.h"

#define SD_CS 5

#define SD_FAT_TYPE 3
#define SPI_SPEED SD_SCK_MHZ(4)

Vars vars;
ArduinoOutStream cout(Serial);

void setup() {
    Serial.begin(115200);
    Serial.println("Setup Finished!");
}

void loop() {
    static Vars *vp = &vars;

    dashboard::fsm(vp, cout);
    logger::fsm(cp, cout);
}