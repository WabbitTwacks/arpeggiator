#include "Button.hpp"

Button::Button(uint8_t pin, void *fncPtr) : Button(pin)
{
  setOnPress(fncPtr);
}

Button::Button(ShiftRegisterIn *shiftReg, uint8_t regBit, void *fncPtr) : Button(shiftReg, regBit)
{
  setOnPress(fncPtr);
}

void Button::setOnPress(void *fncPtr)
{
  onPressFncPtr = fncPtr;
}

void Button::setDebounceTime(uint8_t t)
{
  if (t >= 0)
    debounceSamples = t;
}

void Button::update()
{
  if (inputPin == -1)
    return;

  uint8_t inputValue = HIGH;

  if (shiftRegister == NULL)
    inputValue = digitalRead(inputPin);
  else
    inputValue = shiftRegister->getInput(inputPin);

  if (inputValue == downState && lastState != downState)
  {
    downCount++;
  }

  if (downCount >= debounceSamples)
  {
    downCount = 0;
    upCount = 0;

    if (onPressFncPtr != NULL) //on press
      onPressFncPtr(inputPin);

    lastState = downState;
  }

  if (inputValue != downState && lastState == downState)
    upCount++;

  if (upCount >= debounceSamples)
  {
    upCount = 0;
    downCount = 0;

    if (onReleaseFncPtr != NULL) //on release
      onReleaseFncPtr(inputPin);
    
    lastState = inputValue;
  }  
}
