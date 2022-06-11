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
#include <Smoothed.h>

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
    Smoothed<uint32_t> write_times;

    struct {
        uint32_t num_overflows = 0;
        uint32_t num_writes = 0;
        uint32_t num_write_failures = 0;

        uint32_t t_write_last = 0;
        uint32_t t_write_10_avg = 0;

        uint32_t data_rate_Bps = 0;

        bool logging = false;
    } stats;

    fs::File log_file;
    ArduinoOutStream debug;

    bool new_patch = false;
    StaticJsonDocument<JSON_SIZE> json;
    char filepath[256];
    char filename[256];
    char filebase[30];

    CircularBuffer<uint8_t, SD_BUF_CAPACITY> sd_buf;
    uint8_t write_buf[1024 * 16];

    explicit SDManager(ArduinoOutStream &debug) : debug(debug) {
        write_times.begin(SMOOTHED_AVERAGE, 10);
    };

    void loop() {
        // Check if there is enough outgoing data to write a block.
        if (sd_buf.size() > sizeof(write_buf)) {
            write_buffers();
        }
    }

/*
 * Writes sd_buf into the SD's circular buffer.
 */
    void write_buffers() {
        stats.t_write_last = millis();

        // Copy the FIFO buffer into a contiguous one and write it to log.
        size_t size_read = read_sd_buf(write_buf, sizeof(write_buf));
        if (log_file.write(write_buf, size_read) != size_read) {
            stats.num_write_failures++;
            debug << "ERROR WRITING LOG" << endl;
        } else {
            // Performance tracking
            stats.num_writes++;
            stats.t_write_last = millis() - stats.t_write_last;
            write_times.add(stats.t_write_last);
            stats.t_write_10_avg = write_times.get();
        }
    }

    size_t write(uint8_t *buf, size_t size) {
        if (!is_logging()) return 0;

        size_t i;
        for (i = 0; i < size && sd_buf.available(); i++) {
            sd_buf.push(buf[i]);
        }
        if (i < size) {
            stats.num_overflows += size - i;
        }

        return i;
    }

    /*
     * Reads in a metadata file and adds it to the run database.
     */
    void load_run_metafile(File f) {
        json.clear();
        deserializeJson(json, f);
        json["type"] = "entry";

        append_db();
    }

    /*
     * Creates a run metadata file with the specified entries and
     * adds it to the run database.
     */
    void set_run_metadata(const char *base_name, const char *name,
                          const char *description) {
        json.clear();

        json["type"] = "entry";
        json["file_base"] = base_name;
        json["name"] = name;
        json["description"] = description;

        char path[256];
        snprintf(path, 256, RUNS_DIR"/%s.met", base_name);
        File f = SD.open(path, "w");
        serializeJson(json, f);
        f.close();

        append_db();
    }

    /* Appends the DB with whatever is in this class's "json" var*/
    void append_db() {
        File db = SD.open(RUNS_DB, "a");

        serializeJson(json, db);
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

        stats = {};
        stats.logging = false;
//        stats.num_overflows = 0;
//        stats.num_writes = 0;
//        stats.t_write_last = 0;
//        stats.t_write_avg = 0;
//        stats.data_rate_Bps = 0;

        return true;
    }

    bool is_logging() {
        return stats.logging;
    }

    bool init_log() {
        if (is_logging()) return false;
        stats.logging = true;
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