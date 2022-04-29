//
// Created by Dominic on 4/23/2022.
//
#ifndef SDMANAGER_H
#define SDMANAGER_H

#include "Arduino.h"
#include <SD.h>
#include "PrintMessage.h"
#include "util.h"

#define RUNS_DIR "/r"

#define SD_CS 5

class SDManager {
public:
    fs::File log_file;
    ArduinoOutStream debug;
    bool logging = false;

    char filepath[256];

    uint8_t sd_buf[1024 * 32];
    size_t sd_buf_head;
    size_t sd_buf_tail;
    uint8_t write_buf[1024 * 16];

    explicit SDManager(ArduinoOutStream &debug) : debug(debug) {};

    void loop() {
        if (num_unwritten() > sizeof(write_buf)) {
            size_t size_read = read_sd_buf(write_buf, sizeof(write_buf));
            if (log_file.write(write_buf, size_read) != size_read) {
                debug << "ERROR WRITING LOG" << endl;
            }
        }
    }

/*
 * Writes data into the SD's circular buffer.
 */
    size_t write(uint8_t *buf, size_t size) {
        for (size_t i = 0; i < size; ++i, ++sd_buf_head) {
            sd_buf[sd_buf_head % sizeof(sd_buf)] = buf[i];
        }
        if (sd_buf_head - sd_buf_tail >= sizeof(sd_buf)) sd_buf_tail = sd_buf_head - sizeof(sd_buf) + 1;
        return size;
    }


    void scan_runs() {
        char meta_base[256];
        char meta_path[256];
        uint8_t buf[256];

        fs::File runs = SD.open("/r.txt", FILE_WRITE);
        fs::File run_dir = SD.open(RUNS_DIR);
        fs::File file;

        while ((file = run_dir.openNextFile())) {
            if (get_ext(file.name()) == Ext::CSV) {
                debug << file.name() << endl;
                runs.printf("%s`", file.name());
                get_base(meta_base, file.name(), 256);
                file.close();

                snprintf(meta_path, 256, RUNS_DIR"/%s.met", meta_base);

                file = SD.open(meta_path);

                // Copy the contents of META into r. Should be same format (ie '`' delimited)
                while (file.available() > 0) {
                    size_t size = min(256, file.available());
                    file.read(buf, size);
                    runs.write(buf, size);
                }

                file.close();
                runs.printf("\n");
            }
        }
        run_dir.close();
        runs.flush();
        runs.close();
    }

    bool init() {
        if (!SD.begin(SD_CS, SPI, 40000000))
            return false;

        delay(200);
        if (!SD.exists(RUNS_DIR))
            SD.mkdir(RUNS_DIR);

        scan_runs();
        debug << F("Card initialized!") << endl;
        return true;
    }

    bool close_log() {
        if (!is_logging()) return false;

        while (num_unwritten() > 0) {
            size_t size_read = read_sd_buf(write_buf, sizeof(write_buf));
            log_file.write(write_buf, size_read);
        }

        log_file.close();
        logging = false;
        return true;
    }

    bool is_logging() {
        return logging;
    }

    bool init_log() {
        if (is_logging()) return false;
        logging = true;
        char metafile_path[256];

        //Select next filename
        int file_idx = 0;
        do {
            snprintf(metafile_path, 256, RUNS_DIR"/%d.met", file_idx);
            snprintf(filepath, 256, RUNS_DIR"/%d.csv", file_idx++);
        } while (SD.exists(filepath));


        debug << "Writing data to " << filepath << endl;
        debug << "Writing META to " << metafile_path << endl;

        sd_buf_tail = sd_buf_head;

        File f = SD.open(metafile_path, "w");
        f.printf("Run #%d`No description", file_idx);
        f.close();

        return log_file = SD.open(filepath, "w");
    }

    size_t num_unwritten() {
        return sd_buf_head - sd_buf_tail;
    }


private:

    /*
 * Reads data from the SD's circular buffer. Returns the number of bytes read.
 */
    size_t read_sd_buf(uint8_t *buf, size_t size) {
        size_t i = 0;
        for (; i < size && sd_buf_head - sd_buf_tail > 0; ++i, ++sd_buf_tail) {
            buf[i] = sd_buf[sd_buf_tail % sizeof(sd_buf)];
        }
        return i;
    }
};

#endif