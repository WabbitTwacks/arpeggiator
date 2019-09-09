#include "ShiftRegisterIn.hpp"

ShiftRegisterIn::ShiftRegisterIn()
{
  m_registers = NULL;
}

void ShiftRegisterIn::init(uint8_t loadPin, uint8_t clockEnablePin, uint8_t dataPin, uint8_t clockPin, uint8_t numRegisters = 1)
{
  m_registers = new uint8_t[numRegisters];
  memset(m_registers, 0, numRegisters);

  m_loadPin = loadPin;
  m_clockEnablePin = clockEnablePin;
  m_dataPin = dataPin;
  m_clockPin = clockPin;
  m_numRegisters = numRegisters;

  digitalWrite(m_clockPin, LOW);
  digitalWrite(m_loadPin, HIGH);
}

void ShiftRegisterIn::update()
{
  if (m_registers == NULL)
    return;
  
  digitalWrite(m_clockEnablePin, HIGH);
  digitalWrite(m_loadPin, LOW);
  delayMicroseconds(PULSE_USEC);
  digitalWrite(m_loadPin, HIGH);
  digitalWrite(m_clockEnablePin, LOW);

  //shift in bits
  uint8_t dataWidth = 8 * m_numRegisters;
  uint8_t b = 0;
  memset(m_registers, 0, m_numRegisters);
  
  for (int i = 0; i < dataWidth; i++)
  {
    b = digitalRead(m_dataPin);

    *m_registers |= (b << (dataWidth - 1 - i));

    digitalWrite(m_clockPin, HIGH);
    delayMicroseconds(PULSE_USEC);
    digitalWrite(m_clockPin, LOW);
  }
  m_debug = "";
  for (int i = 0; i < dataWidth; i++)
  {
    m_debug = String((*m_registers>>i) & 1) + m_debug;
  }
}

uint8_t ShiftRegisterIn::getInput(uint8_t input)
{
  if (m_registers == NULL)
    return;
    
  return *m_registers>>input & 1;
}
