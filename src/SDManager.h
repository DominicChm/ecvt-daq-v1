//
// Created by Dominic on 4/23/2022.
//
#ifndef SDMANAGER_H
#define SDMANAGER_H

#include <CircularBuffer.h>
#include "Arduino.h"
#include <SD.h>
#include "PrintMessage.h"
#include "util.h"
#include <ArduinoJson.h>

#define RUNS_DIR "/r"
#define RUNS_DB "/r.jsonl"

#define SD_CS 5

/*
metafile:
 {
    file_base: "run3.csv",
    name: "this is a name",
    desc: "description",
 }
 */


class SDManager {
public:
    CircularBuffer<uint32_t, 10> time_buf;

    struct {
        uint32_t num_overflows;
        uint32_t num_writes;

        uint32_t t_write_last;
        uint32_t t_write_avg;

        uint32_t data_rate_Bps;

        bool logging;
    } status;

    fs::File log_file;
    ArduinoOutStream debug;

    char filepath[256];
    char filename[256];

    CircularBuffer<uint8_t, 1024 * 32> sd_buf;
    uint8_t write_buf[1024 * 16];

    explicit SDManager(ArduinoOutStream &debug) : debug(debug) {};

    void loop() {
        if (sd_buf.available() > sizeof(write_buf)) {
            write_buffers();
        }
    }

/*
 * Writes sd_buf into the SD's circular buffer.
 */
    void write_buffers() {
        status.t_write_last = millis();

        size_t size_read = read_sd_buf(write_buf, sizeof(write_buf));
        if (log_file.write(write_buf, size_read) != size_read) {
            debug << "ERROR WRITING LOG" << endl;
        }

        // Performance tracking
        status.num_writes++;
        status.t_write_last = millis() - status.t_write_last;
        if (status.t_write_last > 0)
            status.data_rate_Bps =
                    sizeof(write_buf) * 1000 / status.t_write_last;
        time_buf.push(status.t_write_last);
        update_time_avg();
    }

    void update_time_avg() {
        if (time_buf.isEmpty()) return;
        status.t_write_avg = 0;
        for (size_t i = 0; i < time_buf.available(); i++) {
            status.t_write_avg += time_buf[i];
        }
        //status.t_write_avg /= time_buf.available();
    }

    size_t write(uint8_t *buf, size_t size) {
        if (!is_logging()) return 0;
        size_t i;
        for (i = 0; i < size && !sd_buf.isFull(); i++) {
            sd_buf.push(buf[i]);
        }

        if (i < size) {
            status.num_overflows += size - i;
        }

        return i;
    }

    void add_db_entry_from_metafile(fs::File metafile, fs::File db) {
        StaticJsonDocument<1024> doc;
        deserializeJson(doc, metafile);

        doc["type"] = "entry";
        serializeJson(doc, db);
        db.println();
    }

    void add_entry_from_metafile(fs::File metafile) {
        File db = SD.open(RUNS_DB, "a");
        add_db_entry_from_metafile(metafile, db);
        db.close();
    }

    void init_run_db() {
        SD.remove(RUNS_DB);

        fs::File run_db = SD.open(RUNS_DB, "a");
        fs::File run_dir = SD.open(RUNS_DIR);
        fs::File file;

        while ((file = run_dir.openNextFile())) {
            if (get_ext(file.name()) != Ext::META) continue;
            add_db_entry_from_metafile(file, run_db);
            file.close();
        }

        run_db.close();
    }

    bool init() {
        if (!SD.begin(SD_CS, SPI, 40000000))
            return false;

        if (!SD.exists(RUNS_DIR))
            SD.mkdir(RUNS_DIR);

        init_run_db();

        debug << F("Card initialized!") << endl;
        return true;
    }

    bool close_log() {
        if (!is_logging()) return false;

        /* Write all unwritten blocks */
        while (num_unwritten() > 0) {
            size_t size_read = read_sd_buf(write_buf, sizeof(write_buf));
            log_file.write(write_buf, size_read);
        }

        log_file.close();

        status = {};
        status.logging = false;
//        status.num_overflows = 0;
//        status.num_writes = 0;
//        status.t_write_last = 0;
//        status.t_write_avg = 0;
//        status.data_rate_Bps = 0;

        return true;
    }

    bool is_logging() {
        return status.logging;
    }

    bool init_log() {
        if (is_logging()) return false;
        status.logging = true;
        char metafile_path[256];
        char filebase[30];
        //Select next filename
        int file_idx = 0;
        do {
            snprintf(metafile_path, 256, RUNS_DIR"/%d.met", file_idx);
            snprintf(filebase, 30, "%d", file_idx++);
            snprintf(filename, 256, "%s.csv", filebase);
            snprintf(filepath, 256, RUNS_DIR"/%s", filename);

        } while (SD.exists(filepath));

        sd_buf.clear();

        debug << "Writing sd_buf to " << filepath << endl;
        debug << "Writing META to " << metafile_path << endl;


        StaticJsonDocument<1024> doc;
        File f = SD.open(metafile_path, "w");

        doc["file_base"] = filebase;
        doc["name"] = "no set name";
        doc["description"] = "no desc";

        serializeJson(doc, f);
        f.flush();

        add_entry_from_metafile(f);

        return log_file = SD.open(filepath, "w");
    }

    size_t num_unwritten() {
        return sd_buf.available();
    }

private:

/*
 * Reads sd_buf from the SD's circular buffer. Returns the number of bytes read.
 */
    size_t read_sd_buf(uint8_t *buf, size_t size) {
        size_t i;
        for (i = 0; i < size && !sd_buf.isEmpty(); i++) {
            buf[i] = sd_buf.pop();
        }
        return i;
    }
};

#endif