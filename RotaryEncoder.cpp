#include "RotaryEncoder.hpp"

void RotaryEncoder::init(uint8_t pinA, uint8_t pinB, uint8_t startPos, 
                         uint8_t minPos, uint8_t maxPos, bool wrapAround,
                         uint8_t debounceTime)
{
    m_pinA = pinA;
    m_pinB = pinB;
    m_debounceTime = debounceTime;
    m_encoderPosition = startPos;
    m_encoderMin = minPos;
    m_encoderMax = maxPos;
    m_wrapAround = wrapAround;

    pinMode(pinA, INPUT_PULLUP);
    pinMode(pinB, INPUT_PULLUP);
}

void RotaryEncoder::setShaftPosition(uint8_t position)
{
  if (position >= m_encoderMin && position <= m_encoderMax)
  {
    m_encoderPosition = position;
  }
}

void RotaryEncoder::setMinValue(uint8_t min)
{
  m_encoderMin = min;
}

void RotaryEncoder::setMaxValue(uint8_t max)
{
  m_encoderMax = max;
}

uint8_t RotaryEncoder::shaftPosition()
{
  return m_encoderPosition;
}

uint8_t RotaryEncoder::trackShaftPosition()
{
  bool pinAValue = digitalRead(m_pinA);
  uint16_t currentTime = millis();
  uint16_t eventsDelta = currentTime - m_lastEventTime;

  if (m_lastA == HIGH && pinAValue == LOW && eventsDelta > m_debounceTime)
  {
    if (digitalRead(m_pinB) == LOW)
    {
      if (m_encoderPosition > m_encoderMin)
      {
          m_encoderPosition--;
      }
      else
      {
        if (m_wrapAround)
          m_encoderPosition = m_encoderMax;
      }
    }
    else
    {
      if (m_encoderPosition < m_encoderMax)
      {
          m_encoderPosition++;
      }
      else
      {
        if (m_wrapAround)
          m_encoderPosition = m_encoderMin;
      }
    }
    m_lastEventTime = currentTime;
  }

  m_lastA = pinAValue;

  return m_encoderPosition;
}

void RotaryEncoder::update()
{
  int pos = trackShaftPosition();

  if (pos == m_lastPosition)
    m_hasChanged = false;
  else
  {
    m_hasChanged = true;
    m_lastPosition = m_encoderPosition;
  }
}
