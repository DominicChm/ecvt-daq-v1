#include "global.h"

/* DAQ FSM state */
global::State global::state;

/* DAQ status */
global::Status global::status;

/* LEDs */
JLed global::daq_led(PIN_DAQ_LED);
JLed global::builtin_led(LED_BUILTIN);

/* Button(s) */
DebouncedButton global::log_btn(PIN_BTN_LOG, 250);

/* Serial */
ArduinoOutStream global::debug(Serial);

/* SD Definitions */
SDManager global::sd_manager(global::debug);

char global::print_buf[512];
