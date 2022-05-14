#include "commparser.h"

static const size_t HEADER_SIZE = 2;
static const size_t TRAILER_SIZE = 2;
static const size_t PAYLOAD_SIZE = sizeof(Data);
static const size_t PACKET_SIZE = sizeof(Data) + HEADER_SIZE + TRAILER_SIZE;

bool parse_fsm(byte b, Data *out_dat) {
    static uint8_t data_buf[PACKET_SIZE];
    static size_t packet_index = 0;

    static enum {
        IDLE,
        READ,
    } state;

    bool flag_reset = false;
    bool flag_successful_parse = false;

    data_buf[packet_index] = b;

    switch (state) {
        case IDLE:
            if (b != DATA_MAGIC_START) {
                flag_reset = true;
            } else if (packet_index >= 1) {
                state = READ;

            }
            break;

        case READ:
            if (packet_index >= PACKET_SIZE - 1) {

                // All packet bytes received - perform checks and send off.
                memcpy(out_dat, &data_buf[HEADER_SIZE], PAYLOAD_SIZE);
                flag_reset = true;
                flag_successful_parse = true;
            }
            break;
    }
    packet_index++;

    if (flag_reset) {
        packet_index = 0;
        state = IDLE;
    }

    return flag_successful_parse;
}

bool parse_data(Data *out_dat) {
    using namespace global;

    if (DATA_SERIAL.available() > 1000)
        status.num_serial_overflows++;

    if (!DATA_SERIAL.available())
        return false;

    status.num_bytes_read++;

    uint8_t b = DATA_SERIAL.read();

    return parse_fsm(b, out_dat);
}
