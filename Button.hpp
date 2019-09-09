#pragma once

#include <Arduino.h>
#include <stdint.h>
#include <limits.h>

#include "ShiftRegisterIn.hpp"

//#define DEFAULT_DEBOUNCE_TIME 10
#define DEFAULT_DEBOUNCE_COUNT 10

class Button
{
  public:
    Button():
      inputPin(-1),
      onPressFncPtr(NULL),
      downState(LOW),
      lastState(HIGH),
      lastDebounce(USHRT_MAX),
      //debounceDelay(DEFAULT_DEBOUNCE_TIME),
      downCount(0),
      upCount(0),
      debounceSamples(DEFAULT_DEBOUNCE_COUNT),
      shiftRegister(NULL)
      {};
    Button(uint8_t pin) : Button() {inputPin = pin;};
    Button(uint8_t pin, void *fncPtr);
    Button(ShiftRegisterIn *shiftReg, uint8_t regBit) : Button() {shiftRegister = shiftReg; inputPin = regBit;};
    Button(ShiftRegisterIn *shiftReg, uint8_t regBit, void *fncPtr);

    void setOnPress(void *fncPtr);
    void setOnRelease(void *fncPtr);
    void setDebounceTime(uint8_t t);

    void update();

  private:
    uint8_t inputPin;
    uint8_t downState;
    uint8_t buttonState;
    uint8_t lastState;
    uint16_t lastDebounce;
    //uint16_t debounceDelay;
    uint8_t downCount;
    uint8_t upCount;
    uint8_t debounceSamples;    
    uint8_t srBit;
    uint8_t srData;
    uint8_t srClock;
    uint8_t srLatch;

    ShiftRegisterIn *shiftRegister;
  
    void (*onPressFncPtr)(uint8_t pin);
    void (*onReleaseFncPtr)(uint8_t pin);
};
