//
// Created by Dominic on 4/23/2022.
//
#ifndef SDMANAGER_H
#define SDMANAGER_H

#include "Arduino.h"
#include <SD.h>
#include "PrintMessage.h"
#define SD_CS 5

template<size_t num_sector_buffers>
class SDManager {
public:
    fs::File log_file;
    ArduinoOutStream debug;
    bool logging = false;

    char filename[256];
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
        fs::File runs_html = SD.open("/r.html", FILE_WRITE);
        runs_html.print("<!doctype html>\n"
                        "<html lang=\"en\">\n"
                        "<head>\n"
                        "<title>Runs</title>\n"
                        "</head>\n"
                        "<body>\n");

        fs::File run_dir = SD.open("/r");
        fs::File f;
        while ((f = run_dir.openNextFile())) {
            debug << f.name() << endl;
            runs_html.printf("<p><a download href=\"/r/%s\">%s</a></p>", f.name(), f.name());
            f.close();
        }
        run_dir.close();

        runs_html.print("</body>");
        runs_html.close();
    }

    bool init() {
        if(!SD.begin(SD_CS, SPI, 40000000))
            return false;

        delay(200);
        if (!SD.exists("/r"))
            SD.mkdir("/r");

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

        return true;
    }

    bool is_logging() {
        return logging;
    }

    bool init_log() {
        if (is_logging()) return false;
        logging = true;
        //Select next filename
        int file_idx = 0;
        do {
            snprintf(filename, 256, "%d.csv", file_idx++);
            snprintf(filepath, 256, "/r/%s", filename);
        } while (SD.exists(filepath));

        debug << "Writing data to " << filepath << endl;

        sd_buf_tail = sd_buf_head;

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