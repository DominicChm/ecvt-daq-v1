//
// Created by Dominic on 4/21/2022.
//

#ifndef ECVT_DAQ_V1_PRINTMESSAGE_H
#define ECVT_DAQ_V1_PRINTMESSAGE_H
namespace PrintMessage {
    void sd_init_failure(ArduinoOutStream &debug) {
        debug << F(
                "\nSD initialization failed.\n"
                "Do not reformat the card!\n"
                "Is the card correctly inserted?\n"
                "Is chipSelect set to the correct value?\n"
                "Does another SPI device need to be disabled?\n"
                "Is there a wiring/soldering problem?\n");
        return;
    }

    void sd_size_failure(ArduinoOutStream &debug) {
        debug << F("Can't determine the card size.\n");
        debug << F("Try another SD card or reduce the SPI bus speed.\n");
        debug << F("Edit SPI_SPEED in this program to change it.\n");
    }

    void sd_web_folder(ArduinoOutStream &debug) {
        debug << "No \"web\" folder found!" << endl;
        debug << "Download the frontend, create \"web\" in the root of the SD, and drop the files in!" << endl;
    }
}
#endif //ECVT_DAQ_V1_PRINTMESSAGE_H
