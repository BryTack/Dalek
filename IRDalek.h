#pragma once

#include <Arduino.h>

// Button codes for the Elegoo-style IR remote
enum IRButton : uint8_t {
  IR_BTN_NONE = 0,
  IR_BTN_0    = 0x19,
  IR_BTN_1    = 0x45,
  IR_BTN_2    = 0x46,
  IR_BTN_3    = 0x47,
  IR_BTN_4    = 0x44,
  IR_BTN_5    = 0x40,
  IR_BTN_6    = 0x43,
  IR_BTN_7    = 0x07,
  IR_BTN_8    = 0x15,
  IR_BTN_9    = 0x09,
  IR_BTN_STAR = 0x16,
  IR_BTN_HASH = 0x0D,
  IR_BTN_OK   = 0x1C,
};

// Initialize the IR receiver on the given pin (default is 12)
void IRDalek_begin(uint8_t receivePin = 12);

// Poll for the last received IR button. Returns IR_BTN_NONE when there is no new command.
IRButton IRDalek_read();
