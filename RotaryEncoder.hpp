#pragma once

#include <Arduino.h>
#include <stdint.h>
#include <limits.h>

#define DEFAULT_DEBOUNCE_TIME 10
#define DEFAULT_MIN_POS 0
#define DEFAULT_MAX_POS 99

class RotaryEncoder
{
  public:
    RotaryEncoder():
      m_pinA(-1),
      m_pinB(-1),
      m_lastA(LOW),
      m_debounceTime(DEFAULT_DEBOUNCE_TIME),
      m_lastEventTime(millis()),
      m_encoderPosition(0),
      m_lastPosition(0),
      m_encoderMax(DEFAULT_MAX_POS),
      m_encoderMin(DEFAULT_MIN_POS),
      m_wrapAround(false)
    {};

    void init(uint8_t pinA, uint8_t pinB, uint8_t startPos = 0, 
              uint8_t minPos = 0, uint8_t maxPos = 99, bool wrapAround = false,
              uint8_t debounceTime = 10);

    void setShaftPosition(uint8_t position);
    void setMinValue(uint8_t min);
    void setMaxValue(uint8_t max);

    uint8_t shaftPosition();

    uint8_t trackShaftPosition();
    void update();
    bool hasChanged() {return m_hasChanged;};

  private:
    uint8_t m_pinA;
    uint8_t m_pinB;
    uint8_t m_lastA;
    uint8_t m_debounceTime;

    uint16_t m_lastEventTime;
    uint8_t m_encoderPosition;
    uint8_t m_encoderMax;
    uint8_t m_encoderMin;
    uint8_t m_lastPosition;

    bool m_wrapAround;
    bool m_hasChanged;
};
