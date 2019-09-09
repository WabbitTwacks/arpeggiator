#include "Arpeggiator.hpp"

uint8_t NoteList::getFirstAvailable()
{
  for (uint8_t i = 0; i < MAX_LIST_SIZE; i++)
  {
    uint8_t index = (i + cache) % MAX_LIST_SIZE;
    
    if (list[index].velocity == 0)
    {
      cache = index;
      return index;
    }
  }
}

void NoteList::add(uint8_t pitch, uint8_t velocity)
{
  if (size < MAX_LIST_SIZE)
    size++;
  else
    return;

  uint8_t index = getFirstAvailable();

  list[index].pitch = pitch;
  list[index].velocity = velocity;
  list[index].prev = last;
  list[index].next = 0;

  list[last].next = index;
  last = index;

  list[index].index = index;
}

void NoteList::remove(uint8_t pitch, bool removeAll)
{
  if (size == 0)
    return;
  
  uint8_t iterator = first;
  uint8_t s = size;

  for (uint8_t i = 0; i < s; i++)
  {
    if (list[iterator].pitch == pitch)
    {
      list[list[iterator].prev].next = list[iterator].next;
      list[list[iterator].next].prev = list[iterator].prev;

      if (iterator == first)
        first = list[iterator].next;
      if (iterator == last)
        last = list[iterator].prev;

      list[iterator].velocity = 0;

      size--;

      if (!removeAll)
        return;
    }

    iterator = list[iterator].next;
  }
}

/*void NoteList::remove(ArpNote *n)
{
  ArpNote *iterator = first;

  while(iterator != NULL)
  {
    ArpNote *next = iterator->next;

    if (iterator == n)
    {
      if (iterator == first)
        removeFirst();
      else if (iterator == last)
        removeLast();
      else
      {     
        if (iterator->prev != NULL)
          iterator->prev->next = iterator->next;
        if (iterator->next != NULL)
          iterator->next->prev = iterator->prev;
  
        delete iterator;
  
        size--;
      }
    }

    iterator = next;
  }
}*/

/*void NoteList::removeLast()
{
  if (first == last)
    first = NULL;
    
  if (last != NULL)
  {
    ArpNote *t = NULL;
    
    if (last->prev != NULL)
    {
      t = last->prev;
      t->next = NULL;
    }  
    
    delete last;
    last = t;

    size--;
  }
}

void NoteList::removeFirst()
{
  if (first == last)
    last = NULL;
    
  if (first != NULL)
  {
    ArpNote *t = NULL;
    
    if (first->next != NULL)
    {
      t = first->next;
      t->prev = NULL;
    }  
    
    delete first;
    first = t;

    size--;
  }
}*/

void NoteList::clear()
{
    for (uint8_t i = 0; i < MAX_LIST_SIZE; i++)
    {
      list[i].velocity = 0;
    }

    first = 0;
    last = 0;
    size = 0;
    cache = 0;
}

ArpNote NoteList::getLowestNote()
{
  uint8_t lowest = first;
  uint8_t iterator = first;

  for (uint8_t i = 0; i < size; i++)
  {
    if (list[iterator].pitch <= list[lowest].pitch)
      lowest = iterator;

    iterator = list[iterator].next;
  }

  return list[lowest];
}

/*NoteList & NoteList::operator=(const NoteList &c)
{
  if (this == &c)
    return *this;

  clear();
  
  ArpNote *iterator = c.getFirst();

  while (iterator != NULL)
  {
    add(iterator->pitch, iterator->velocity);

    iterator = iterator->next;
  }

  return *this;
}*/

ArpNote &NoteList::operator[](uint8_t index)
{
  uint8_t iterator = first;

  for (uint8_t i = 0; i < size; i++)
  {
    if (i == index)
    {
      return list[iterator];
    }

    iterator = list[iterator].next;
  }
}

void Arpeggiator::generateArpeggio(uint8_t alg, uint8_t oct)
{
  NoteList temp = origin;

  ArpNote note = {0, 0, 0, 0};
  uint8_t s = temp.getSize();
  
  switch(alg)
  {
    case ARP_UP:
    {      
       for (uint8_t i = 0; i < s; i++)
       {
          note = temp.getLowestNote();
  
          for (uint8_t j = 0; j < oct; j++)
          {
            if ((i + (j*s)) >= MAX_ARP_SIZE)
              break;
            
            arpeggio[i + (j*s)].pitch = note.pitch + (j*12);
            arpeggio[i + (j*s)].velocity = note.velocity;
          }
  
          temp.remove(note.pitch, false);
       }    

       arpLength = min(s * oct, MAX_ARP_SIZE);
      
      break;
    }
    case ARP_DOWN:
    {      
       for (uint8_t i = 0; i < s; i++)
       {
          note = temp.getLowestNote();
  
          for (uint8_t j = 0; j < oct; j++)
          {
            if (((s - i - 1) + (j*s)) >= MAX_ARP_SIZE)
              break;
              
            arpeggio[(s - i - 1) + (j*s)].pitch = note.pitch - (j*12);
            arpeggio[(s - i - 1) + (j*s)].velocity = note.velocity;
          }
  
          temp.remove(note.pitch, false);
       }

       arpLength = min(s * oct, MAX_ARP_SIZE);
     
      break;
    }
    case ARP_UP_DOWN:
    {
      for (uint8_t i = 0; i < s; i++)
       {
          note = temp.getLowestNote();
  
          for (uint8_t j = 0; j < oct; j++)
          {
            if ((i + ((s*2-2)*j)) >= MAX_ARP_SIZE)
              break;

            arpeggio[i + ((s*2-2)*j)].pitch = note.pitch + (j*12);
            arpeggio[i + ((s*2-2)*j)].velocity = note.velocity;

            if (i == 0 || i == (s-1))
              continue;

            if (((s*2 - i - 2) + ((s*2-2)*j)) >= MAX_ARP_SIZE)
              break;
              
            arpeggio[(s*2 - i - 2) + ((s*2-2)*j)].pitch = note.pitch + (j*12);
            arpeggio[(s*2 - i - 2) + ((s*2-2)*j)].velocity = note.velocity;
          }
  
          temp.remove(note.pitch, false);
       }

       arpLength = min((s*2 - 2) * oct, MAX_ARP_SIZE);
       arpLength = max(arpLength, oct);

       break;
    }
    case ARP_DOWN_UP:
    {
      for (uint8_t i = 0; i < s; i++)
       {
          note = temp.getLowestNote();
  
          for (uint8_t j = 0; j < oct; j++)
          {
            if (((s - i - 1) + ((s*2-2)*j)) >= MAX_ARP_SIZE)
              break;
              
            arpeggio[((s - i - 1) + ((s*2-2)*j))].pitch = note.pitch - (j*12);
            arpeggio[((s - i - 1) + ((s*2-2)*j))].velocity = note.velocity;

            if (i == 0 || i == (s-1))
              continue;

            if ((i + s - 1 + ((s*2-2)*j)) >= MAX_ARP_SIZE)
              break;

            arpeggio[i + s - 1 + ((s*2-2)*j)].pitch = note.pitch - (j*12);
            arpeggio[i + s - 1 + ((s*2-2)*j)].velocity = note.velocity;
          }
  
          temp.remove(note.pitch, false);
       }

       arpLength = min((s*2 - 2) * oct, MAX_ARP_SIZE);
       arpLength = max(arpLength, oct);

       break;
    }
    case ARP_RANDOM:
    case ARP_RANDOM2:
    {
       for (uint8_t i = 0; i < s; i++)
       {
          note = temp[random(temp.getSize())];
  
          for (uint8_t j = 0; j < oct; j++)
          {
            if ((i + (j*s)) >= MAX_ARP_SIZE)
              break;
            
            arpeggio[i + (j*s)].pitch = note.pitch + (j*12);
            arpeggio[i + (j*s)].velocity = note.velocity;
          }
  
          temp.remove(note.pitch, false);
       }    

       arpLength = min(s * oct, MAX_ARP_SIZE);
      
      break;
    }
    case ARP_SEQUENCE:
    {
       for (uint8_t i = 0; i < s; i++)
       {
          note = temp[i];
  
          for (uint8_t j = 0; j < oct; j++)
          {
            if ((i + (j*s)) >= MAX_ARP_SIZE)
              break;
            
            arpeggio[i + (j*s)].pitch = note.pitch + (j*12);
            arpeggio[i + (j*s)].velocity = note.velocity;
          }
       }    

       arpLength = min(s * oct, MAX_ARP_SIZE);
      
      break;
    }
  }  
}

MidiNote Arpeggiator::getNote(bool randomNote)
{
  if (randomNote)
    return arpeggio[random(arpLength)];
  
  if (iterator >= arpLength)
    iterator = 0;
  
  MidiNote r = arpeggio[iterator];
  
  iterator++;

  if (iterator >= arpLength)
    iterator = 0;

  return r;
}

/*void Arpeggiator::stopNote(uint8_t channel)
{
  uint8_t i = iterator-1;

  if (i < 0)
    i = arpeggio.getSize()-1;
  
  MIDI.sendNoteOff(arpeggio[i].pitch, 0, channel);
}*/

String Arpeggiator::dump()
{
  String d = "";

  for (uint8_t i = 0; i < origin.getSize(); i++)
  {
   d += String(origin[i].index) + ":" + String(origin[i].pitch) + ((i != origin.getSize()-1)?", ":"");
  // d = "last: " + String(origin.getLast().index);
  }

  return d;
}
