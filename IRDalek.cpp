#include <Arduino.h>
#include <IRremote.hpp>

#include "IRDalek.h"

namespace {
IRButton lastButton = IR_BTN_NONE;
constexpr uint8_t IR_DEFAULT_RECEIVE_PIN = 12;

bool isKnownButton(uint8_t cmd) {
  switch (cmd) {
    case IR_BTN_0:
    case IR_BTN_1:
    case IR_BTN_2:
    case IR_BTN_3:
    case IR_BTN_4:
    case IR_BTN_5:
    case IR_BTN_6:
    case IR_BTN_7:
    case IR_BTN_8:
    case IR_BTN_9:
    case IR_BTN_STAR:
    case IR_BTN_HASH:
    case IR_BTN_OK:
      return true;
    default:
      return false;
  }
}
}

void IRDalek_begin(uint8_t receivePin) {
  if (receivePin == 0) {
    receivePin = IR_DEFAULT_RECEIVE_PIN;
  }
  IrReceiver.begin(receivePin, DISABLE_LED_FEEDBACK);
  lastButton = IR_BTN_NONE;
}

IRButton IRDalek_read() {
  // Check hardware for a new command
  if (IrReceiver.decode()) {
    if (IrReceiver.decodedIRData.protocol == NEC) {
      uint8_t cmd = IrReceiver.decodedIRData.command;
      if (isKnownButton(cmd)) {
        lastButton = static_cast<IRButton>(cmd);
      }
    }
    // Ready for the next signal regardless of what we saw
    IrReceiver.resume();
  }

  IRButton result = lastButton;
  lastButton      = IR_BTN_NONE; // clear the cache once returned
  return result;
}
