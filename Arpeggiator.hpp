#pragma once

#include <MIDI.h>
#include <Arduino.h>
#include <stdint.h>

#define MAX_LIST_SIZE 25
#define MAX_ARP_SIZE MAX_LIST_SIZE*2

enum algorithms {ARP_UP, ARP_DOWN, ARP_UP_DOWN, ARP_DOWN_UP, ARP_RANDOM, ARP_RANDOM2, ARP_SEQUENCE};

struct ArpNote
{
  uint8_t pitch;
  uint8_t velocity;
  uint8_t next;
  uint8_t prev;
  uint8_t index;
};

struct MidiNote
{
  uint8_t pitch = 0;
  uint8_t velocity = 0;
};

class NoteList
{
  public:
    NoteList():
      first(0),
      last(0),
      size(0),
      cache(0){};

    void add(uint8_t pitch, uint8_t velocity);
    void remove(uint8_t pitch, bool removeAll = true);
    //void remove(ArpNote *n);
    //void removeLast();
    //void removeFirst();
    void clear();

    ArpNote getLowestNote();
    ArpNote getFirst() {return list[first];};
    ArpNote getLast() {return list[last];};

    uint8_t getSize() {return size;};

    //NoteList & operator=(const NoteList &c);
    ArpNote &operator[](uint8_t index);

  private:
    uint8_t first;
    uint8_t last;

    ArpNote list[MAX_LIST_SIZE];

    uint8_t size;

    uint8_t cache;

    uint8_t getFirstAvailable();
};

class Arpeggiator
{
  public:
    Arpeggiator():
      iterator(0),
      arpLength(0) {};

    void generateArpeggio(uint8_t alg, uint8_t oct);
    MidiNote getNote(bool randomNote = false);
    uint8_t getSize() {return arpLength;};
    //void stopNote(uint8_t channel);

    void add(uint8_t pitch, uint8_t velocity) {origin.add(pitch, velocity);};
    void remove(uint8_t pitch) {origin.remove(pitch);};
    void clear() {/*arpeggio.clear();*/ origin.clear(); arpLength = 0;};
    void resetIterator() {iterator = 0;};

    String dump();

  private:
    NoteList origin;
    MidiNote arpeggio[MAX_ARP_SIZE];

    uint8_t iterator;
    uint8_t arpLength;
};
