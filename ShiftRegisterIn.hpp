#pragma once

#include <Arduino.h>
#include <stdint.h>

#define PULSE_USEC 5

class ShiftRegisterIn
{
  public:
    ShiftRegisterIn();

    void init(uint8_t loadPin, uint8_t clockEnablePin, uint8_t dataPin, uint8_t clockPin, uint8_t numRegisters = 1);

    void update();
    uint8_t getInput(uint8_t input); //first input is 0

    String debug() {return m_debug;};
    
  private:
    uint8_t *m_registers;
    uint8_t m_loadPin;
    uint8_t m_clockEnablePin;
    uint8_t m_dataPin;
    uint8_t m_clockPin;
    uint8_t m_numRegisters;

    String m_debug;
};
