//
// Created by Dominic on 4/23/2022.
//
#ifndef SDMANAGER_H
#define SDMANAGER_H

#include <CircularBuffer.h>
#include "Arduino.h"
#include <SD.h>
#include "util.h"
#include <ArduinoJson.h>

#define RUNS_DIR "/r"
#define RUNS_DB "/r.jsonl"

#define JSON_SIZE 1024
#define SD_CS 5

#define SD_BUF_CAPACITY 1024 * 32
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
        uint32_t num_overflows = 0;
        uint32_t num_writes = 0;

        uint32_t t_write_last = 0;
        uint32_t t_write_avg = 0;

        uint32_t data_rate_Bps = 0;

        bool logging = false;
    } status;

    fs::File log_file;
    ArduinoOutStream debug;

    bool new_patch = false;
    StaticJsonDocument<JSON_SIZE> last_db_entry;
    char filepath[256];
    char filename[256];
    char filebase[30];

    CircularBuffer<uint8_t, SD_BUF_CAPACITY> sd_buf;
    uint8_t write_buf[1024 * 16];

    explicit SDManager(ArduinoOutStream &debug) : debug(debug) {};

    void loop() {
        if (sd_buf.size() > sizeof(write_buf)) {
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
            status.data_rate_Bps = sizeof(write_buf) * 1000 / status.t_write_last;
        time_buf.push(status.t_write_last);
        update_time_avg();
    }

    void update_time_avg() {
        if (!time_buf.size()) return;
        status.t_write_avg = 0;
        for (size_t i = 0; i < time_buf.size(); i++) {
            status.t_write_avg += time_buf[i];
        }
        status.t_write_avg /= time_buf.size();
    }

    size_t write(uint8_t *buf, size_t size) {
        if (!is_logging()) return 0;

        size_t i;
        for (i = 0; i < size && sd_buf.available(); i++) {
            sd_buf.push(buf[i]);
        }
        if (i < size) {
            status.num_overflows += size - i;
        }

        return i;
    }

    /*
     * Reads in a metadata file and adds it to the run database.
     */
    void load_run_metafile(File f) {
        last_db_entry.clear();
        deserializeJson(last_db_entry, f);
        append_db();
    }

    /*
     * Creates a run metadata file with the specified entries and
     * adds it to the run database.
     */
    void set_run_metadata(const char *base_name, const char *name, const char *description) {
        last_db_entry.clear();

        last_db_entry["type"] = "entry";
        last_db_entry["file_base"] = base_name;
        last_db_entry["name"] = name;
        last_db_entry["description"] = description;

        char path[256];
        snprintf(path, 256, RUNS_DIR"/%s.met", base_name);
        File f = SD.open(path, "w");
        serializeJson(last_db_entry, f);
        f.close();

        append_db();
    }

    /* Appends the DB with last_patch */
    void append_db() {
        File db = SD.open(RUNS_DB, "a");
        StaticJsonDocument<JSON_SIZE> doc;

        serializeJson(doc, db);
        db.println();
        db.close();
        new_patch = true;
    }

    /*
     * Initializes the run database file.
     * Essentially concats all JSON metadata with newlines to create a "jsonl"
     * (line-delimited-json) file. New runs, or changes to old run meta, are
     * appended to this file as patches during runtime for performance.
     * This function can take a LONG time to execute, so use calls sparingly.
     */
    void init_run_db() {
        debug << "Initing DB!" << endl;
        SD.remove(RUNS_DB);

        fs::File run_dir = SD.open(RUNS_DIR);
        fs::File file;

        while ((file = run_dir.openNextFile())) {
            if (get_ext(file.name()) != Ext::META) continue;

            load_run_metafile(file);
            file.close();
        }

        run_dir.close();
    }

    /*
     * Initializes the SD card
     */
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
        debug << "Closing log!" << endl;
        /* Write all unwritten blocks */
        while (num_unwritten() > 0) {
            size_t size_read = read_sd_buf(write_buf, sizeof(write_buf));
            log_file.write(write_buf, size_read);
        }
        debug << "Last blocks written!" << endl;
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
        //Select next filename
        int file_idx = 0;
        do {
            snprintf(filebase, 30, "%d", file_idx++);
            snprintf(filename, 256, "%s.csv", filebase);
            snprintf(filepath, 256, RUNS_DIR"/%s", filename);

        } while (SD.exists(filepath));

        sd_buf.clear();

        debug << "Writing data to " << filepath << endl;

        set_run_metadata(filebase, "No name", "No desc");

        return log_file = SD.open(filepath, "w");
    }

    size_t num_unwritten() {
        return sd_buf.size();
    }

private:

/*
 * Reads sd_buf from the SD's circular buffer. Returns the number of bytes read.
 */
    size_t read_sd_buf(uint8_t *buf, size_t size) {
        size_t i;
        for (i = 0; i < size && sd_buf.size(); i++) {
            buf[i] = sd_buf.shift();
        }
        return i;
    }
};

#endif