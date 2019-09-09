#include <MIDI.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "ShiftRegisterIn.hpp"
#include "Button.hpp"
#include "RotaryEncoder.hpp"
#include "Arpeggiator.hpp"

MIDI_CREATE_DEFAULT_INSTANCE();

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define PPQN 24
#define PPQN4 PPQN*4
#define MIDI_CHANNEL 1
#define LAST_NOTEON_TIME 100
#define BUTTON_DEBOUNCE 20

#define LED 13
#define SRCLKEN_PIN 7
#define SRDATA_PIN 8
#define SRCLOCK_PIN 9
#define SRLOAD_PIN 10

#define ENC_NOTE_PINA 3
#define ENC_NOTE_PINB 4

#define ENC_ALG_PINA 5
#define ENC_ALG_PINB 6

#define STARTING_NOTE_LENGTH 3 //index of noteLengthsMap
#define OCTAVE_MAX 4

uint8_t noteLengthsMap[7] = {1, 2, 4, 8, 16, 32, 64};
const char *algNames[] = {"UP", "DOWN", "UP+DOWN", "DOWN+UP", "RANDOM", "RANDOM2", "SEQUENCE"};

uint8_t nuMidiNotesDown = 0;
MidiNote currentNote;

uint8_t algorithm = 0;

bool active = false;
bool ledOn = false;
bool localOn = true;

byte localDebouncer = 0;

uint16_t pulse = -1;
bool freshPulse = false;
byte startPulse = 0;
bool noteOn = false;
byte noteLength = noteLengthsMap[STARTING_NOTE_LENGTH]; //1, 2, 4, 8, 16, 32
byte gateLength = 95; //1 - 95
uint8_t octave = 1;

unsigned long lastNoteOnTime = 0;

bool updateDisplay = false;

//extra params
bool resetIterator = false;
bool latch = true;

uint8_t noteMode = 0; //0 - normal, 1 - tripplet, 2 - dotted

//Buttons and interface
ShiftRegisterIn shiftRegister;

Button buttonOnOff(&shiftRegister, 0);
Button buttonOctDown(&shiftRegister, 1);
Button buttonOctUp(&shiftRegister, 2);
Button buttonLatch(&shiftRegister, 3);
Button buttonResetArp(&shiftRegister, 4);
Button buttonTripDot(&shiftRegister, 5);

RotaryEncoder encoderNote;
RotaryEncoder encoderAlg;

Arpeggiator arp;

String debug = "";

void resetDisplay()
{
  digitalWrite(OLED_RESET, LOW);
  delay(1000);
  digitalWrite(OLED_RESET, HIGH);
}

void clearNotes()
{
  arp.clear();
}

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
  if (localOn)
    return;

  if (nuMidiNotesDown == 0)
  {
      clearNotes();
      pulse = 0;
  }

  nuMidiNotesDown++;
  updateDisplay = true;
  arp.add(pitch, velocity);
  //debug = arp.dump();
  arp.generateArpeggio(algorithm, octave);

  //if (resetIterator)
    arp.resetIterator();
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
  if (localOn)
    return;
    
  if (!latch)
  {
    //notesDown[pitch] = false;
    arp.remove(pitch);
    arp.generateArpeggio(algorithm, octave);

    if (resetIterator)
      arp.resetIterator();
  }

  nuMidiNotesDown--;

  if (!latch && nuMidiNotesDown == 0)
    MIDI.sendControlChange(123, 0, MIDI_CHANNEL); //all notes off
  //updateDisplay = true;
}

void handleClock()
{
  pulse++;
  if (pulse >= 720)
    pulse = 0;

  freshPulse = true;
  
  //digitalWrite(LED, !ledOn);
  //ledOn = !ledOn;  
}

//callbacks for buttons etc.
void toggleOnOff(uint8_t pin)
{
  if (!localOn)
  {
    clearNotes();
    freshPulse = false;

    MIDI.sendControlChange(123, 0, MIDI_CHANNEL); //all notes off
    nuMidiNotesDown = 0;
  }
  
  localOn = !localOn;
     
  digitalWrite(LED, !localOn);    

  MIDI.sendControlChange(122, localOn?127:0, MIDI_CHANNEL); //local sw on/off
}

void octaveDown(uint8_t pin)
{
  octave--;

  if (octave < 1)
    octave = OCTAVE_MAX;

  arp.generateArpeggio(algorithm, octave);

  updateDisplay = true;
}


void octaveUp(uint8_t pin)
{
  octave++;

  if (octave > OCTAVE_MAX)
    octave = 1;

  arp.generateArpeggio(algorithm, octave);

  updateDisplay = true;
}

void toggleLatch(uint8_t pin)
{
  latch = !latch;
  updateDisplay = true;
}

void arpReset(uint8_t pin)
{
  arp.resetIterator();

  if (algorithm == ARP_RANDOM2)
    arp.generateArpeggio(ARP_RANDOM2, octave);
}

void switchNoteMode(uint8_t pin)
{
  noteMode++;

  if (noteMode > 3)
    noteMode = 0;

  updateDisplay = true;
}

void redrawDisplay()
{
  display.clearDisplay();
  
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setTextWrap(false);
  
  //Algorithm selection
  display.setCursor(0, 0);
  display.print(F("ALG:"));
  display.print(algNames[algorithm]);

  //Octave
  display.setCursor(0, 12);
  display.print(F("OCT:")); display.print(octave);

  //Latch
  display.setCursor(0, 24);
  //display.setTextColor(BLACK, WHITE);
  display.print(F("LATCH:"));
  display.print(latch?F("ON"):F("OFF"));
  display.setTextColor(WHITE, BLACK);

  //Debug info
  //display.setCursor(0, 24);
  //display.print(debug);

  //Note length
  const uint8_t note_abs_x = 82;
  const uint8_t note_abs_y = 0;
  
  display.setTextSize(2);
  display.setCursor(note_abs_x, note_abs_y);
  display.print(F("1")); 
  display.setCursor(note_abs_x + 12, note_abs_y + 16);
  display.print(noteLengthsMap[encoderNote.shaftPosition()]);
  display.drawLine(note_abs_x + 2, note_abs_y + 22, note_abs_x + 18, note_abs_y + 6, WHITE);

  display.setCursor(note_abs_x + 24, note_abs_y + 0);
  if (noteMode == 1)
  {    
    display.print(F("T"));
  }
  else if (noteMode == 2)
  {
    display.print(F("D"));
  }
  else if (noteMode == 3)
  {
    display.print(F("Q"));
  }
   
  display.display();
}

void setup() {
  randomSeed(analogRead(0));
  
  // put your setup code here, to run once:
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

   //delay(1000);
   resetDisplay();
   //redrawDisplay();   
  
  pinMode(LED, OUTPUT);
  pinMode(SRDATA_PIN, INPUT);
  pinMode(SRCLOCK_PIN, OUTPUT);
  pinMode(SRLOAD_PIN, OUTPUT);
  pinMode(SRCLKEN_PIN, OUTPUT);

  shiftRegister.init(SRLOAD_PIN, SRCLKEN_PIN, SRDATA_PIN, SRCLOCK_PIN);

  buttonOnOff.setOnPress(toggleOnOff);
  buttonOnOff.setDebounceTime(BUTTON_DEBOUNCE);

  buttonOctDown.setOnPress(octaveDown);
  buttonOctDown.setDebounceTime(BUTTON_DEBOUNCE);

  buttonOctUp.setOnPress(octaveUp);
  buttonOctUp.setDebounceTime(BUTTON_DEBOUNCE);

  buttonLatch.setOnPress(toggleLatch);
  buttonLatch.setDebounceTime(BUTTON_DEBOUNCE);

  buttonResetArp.setOnPress(arpReset);
  buttonResetArp.setDebounceTime(BUTTON_DEBOUNCE);

  buttonTripDot.setOnPress(switchNoteMode);
  buttonTripDot.setDebounceTime(BUTTON_DEBOUNCE);

  encoderNote.init(ENC_NOTE_PINA, ENC_NOTE_PINB, STARTING_NOTE_LENGTH, 0, 5, true, 50);
  encoderAlg.init(ENC_ALG_PINA, ENC_ALG_PINB, algorithm, 0, 6, true, 50);
  
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandleClock(handleClock);
  MIDI.begin(MIDI_CHANNEL);
  MIDI.turnThruOff(); 

  //clear data
  clearNotes();
}

void loop() {
 
 shiftRegister.update(); 
 buttonOnOff.update();
 buttonOctDown.update();
 buttonOctUp.update();
 buttonLatch.update();
 buttonResetArp.update();
 buttonTripDot.update();
 encoderNote.update();
 encoderAlg.update();

 if (encoderNote.hasChanged())
 {
    noteLength = noteLengthsMap[encoderNote.shaftPosition()];
    updateDisplay = true;
 }

 if (encoderAlg.hasChanged())
 {
    algorithm = encoderAlg.shaftPosition();
    arp.generateArpeggio(algorithm, octave);
    updateDisplay = true;
 }
 
 uint8_t divider = (PPQN4) / noteLength;
 
 //byte sendPulse = startPulse % divider;

 if (noteMode == 1) //tripplets
    divider = divider*2/3;
 else if (noteMode == 2) //dotted
    divider = divider + divider/2;
 else if (noteMode == 3) //quintuplets (but not precise because of resolution)
 {    
    if (pulse % (divider*4) == startPulse % (divider*4))
    {
      divider = (divider*4/5) + (divider*4)%5; //align first beat
    }
    else
    {
      divider = divider*4 / 5;
    }
 }

 gateLength = divider - 1;

  MIDI.read();
  
 //send arp notes
 if (freshPulse && !localOn && arp.getSize())
 {
   if (pulse % divider == 1) //send note
   {
      currentNote = arp.getNote(algorithm == ARP_RANDOM);
      MIDI.sendNoteOn(currentNote.pitch, currentNote.velocity, MIDI_CHANNEL);
   }
   else if (pulse % divider == gateLength) //stop note\\\\\\\\\\\\\\\\
   {
      MIDI.sendNoteOff(currentNote.pitch, 0, MIDI_CHANNEL);
   }

   freshPulse = false;
 }

 if (updateDisplay)
 {
    redrawDisplay();
    updateDisplay = false;
 }
}
