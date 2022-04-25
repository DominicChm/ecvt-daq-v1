//
// Created by Dominic on 4/23/2022.
//
#ifndef SDMANAGER_H
#define SDMANAGER_H

#include "Arduino.h"

#define SD_CS 5
#define SPI_SPEED SD_SCK_MHZ(4)

template<size_t num_sector_buffers>
class SDManager {
public:
    SdFs sd;
    FsFile log_file;
    ArduinoOutStream debug;

    char filename[256];
    char filepath[256];

    explicit SDManager(ArduinoOutStream &debug) : debug(debug) {};

    void loop() {
        if (!sd.isBusy()) {
            size_t size_read = read_sd_buf(write_buf, sizeof(write_buf));
            if (size_read > 0) {
                log_file.write(write_buf, size_read);
            }
        }
    }

/*
 * Writes data into the SD's circular buffer.
 */
    size_t write(uint8_t *buf, size_t size) {
        size_t i = 0;
        for (; i < size && (sd_buf_tail - sd_buf_head) % sizeof(sd_buf) != 1;
               i++, ++sd_buf_head %= sizeof(sd_buf)) {
            sd_buf[sd_buf_head] = buf[i];
        }
        return i;
    }

    bool init() {
        if (!sd.begin(SD_CS, SPI_SPEED)) {
            PrintMessage::sd_init_failure(debug);
            return false;
        }

        if (sd.card()->sectorCount() == 0) {
            PrintMessage::sd_size_failure(debug);
            return false;
        }

        // Check folders
        if (!sd.exists("web")) {
            PrintMessage::sd_web_folder(debug);
            return false;
        }

        if (!sd.exists("runs"))
            sd.mkdir("runs");

        debug << F("Card initialized!") << endl;
        return true;
    }


private:
    static uint8_t sd_buf[512 * num_sector_buffers];
    static size_t sd_buf_head;
    static size_t sd_buf_tail;
    static uint8_t write_buf[512];

    /*
 * Reads data from the SD's circular buffer. Returns the number of bytes read.
 */
    size_t read_sd_buf(uint8_t *buf, size_t size) {
        size_t i = 0;
        for (; i < size && sd_buf_head - sd_buf_tail > 0;
               i++, ++sd_buf_tail %= sizeof(sd_buf)) {
            buf[i] = sd_buf[sd_buf_tail];
        }
        return i;
    }

    bool init_log() {
        if (log_file.isOpen()) return false;

        //Select next filename
        int file_idx = 0;
        do {
            snprintf(filename, 256, "ecvtdat%d.csv", file_idx++);
            snprintf(filepath, 256, "/runs/%s", filename);
        } while (sd.exists(filepath));

        debug << "Writing data to " << filepath << endl;

        log_file.open(filename, O_RDWR | O_CREAT);
        return true;
    }

    bool close_log() {
        if (!log_file.isOpen()) return false;

        file.flush();
        file.truncate();
        file.sync();
        file.close();
    }
};

#endif