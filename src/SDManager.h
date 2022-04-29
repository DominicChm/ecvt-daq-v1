//
// Created by Dominic on 4/23/2022.
//
#ifndef SDMANAGER_H
#define SDMANAGER_H

#include "Arduino.h"

#define SD_CS 5
#define SPI_SPEED SD_SCK_MHZ(25)

template<size_t num_sector_buffers>
class SDManager {
public:
    SdFs sd;
    FsFile log_file;
    ArduinoOutStream debug;
    bool logging = false;

    char filename[256];
    char filepath[256];

    uint8_t sd_buf[512 * num_sector_buffers];
    size_t sd_buf_head;
    size_t sd_buf_tail;
    uint8_t write_buf[512];

    explicit SDManager(ArduinoOutStream &debug) : debug(debug) {};

    void loop() {
        if (!sd.isBusy() && num_unwritten() > 512) {
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

    bool guard_sdop() {
        return sd.begin(SD_CS, SPI_SPEED);
    }

    void scan_runs() {
        char filename[256];
        FsFile runs = sd.open("runs.txt", O_RDWR | O_CREAT);
        FsFile runs_html = sd.open("r", O_RDWR | O_CREAT);
        runs_html.print("<!doctype html>\n"
                        "<html lang=\"en\">\n"
                        "<head>\n"
                        "<title>Runs</title>\n"
                        "</head>\n"
                        "<body>\n");

        FsFile run_dir = sd.open("/runs/");
        FsFile f;
        while ((f = run_dir.openNextFile())) {
            f.getName(filename, 256);
            f.close();
            debug << filename << endl;
            runs.print(filename);
            runs.print(",");
            runs_html.printf("<p><a download href=\"/runs/%s\">%s</a></p>", filename, filename);
        }
        runs_html.print("</body>");

        runs_html.sync();
        runs_html.truncate();
        runs_html.close();

        runs.sync();
        runs.truncate();
        runs.close();

        run_dir.close();

    }

    bool init() {
        if (!guard_sdop()) {
            PrintMessage::sd_init_failure(debug);
            return false;
        }

        if (sd.card()->sectorCount() == 0) {
            PrintMessage::sd_size_failure(debug);
            return false;
        }

        if (!sd.exists("runs"))
            sd.mkdir("runs");

        scan_runs();
        debug << F("Card initialized!") << endl;
        return true;
    }

    bool close_log() {
        if (!guard_sdop()) return false;
        if (!is_logging()) return false;

        while (num_unwritten() > 0) {
            size_t size_read = read_sd_buf(write_buf, sizeof(write_buf));
            log_file.write(write_buf, size_read);
        }

        log_file.truncate();
        log_file.sync();
        log_file.close();

        return true;
    }

    bool is_logging() {
        return logging;
    }

    bool init_log() {
        if (!guard_sdop()) return false;
        if (is_logging()) return false;
        logging = true;
        //Select next filename
        int file_idx = 0;
        do {
            snprintf(filename, 256, "ecvtdat%d.csv", file_idx++);
            snprintf(filepath, 256, "/runs/%s", filename);
        } while (sd.exists(filepath));

        debug << "Writing data to " << filepath << endl;

        sd_buf_tail = sd_buf_head;

        log_file.open(filepath, O_RDWR | O_CREAT);

        memset(write_buf, 0, sizeof(write_buf));
        if (log_file.write(write_buf, 512) != 512) {
            debug << "write first sector failed" << endl;
        }

        log_file.preAllocate(3000);
        return log_file.isOpen();
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