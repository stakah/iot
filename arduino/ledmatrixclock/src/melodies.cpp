#include <Arduino.h>
#include "melodies.h"
#ifndef __MELODIES__CPP
#define __MELODIES__CPP

note happy[] =     // Happy birthday
    {
        {8,ND4}, {8,ND4}, {4,NE4}, {4,ND4}, {4,NG4}, {2,NF4},
        {8,ND4}, {8,ND4}, {4,NE4}, {4,ND4}, {4,NA4}, {2,NG4},
        {8,ND4}, {8,ND4}, {4,ND5}, {4,NB4}, {4,NG4}, {4,NF4}, {4,NE4},
        {8,NC5}, {8,NC5}, {4,NB4}, {4,NG4}, {4,NA4}, {2,NG4},
        {4,NR}
    };

note auld[] =     // Ald Lang
    {
        {4,  NC4}, {8/3,NF4}, {8,  NF4}, {4,  NF4}, {4,  NA4}, {8/3,NG4},
        {8,  NF4}, {4,  NG4}, {8,  NA4}, {8,  NG4}, {8/3,NF4}, {8,  ND4},
        {4,  ND4}, {4,  NC4}, {4/3,NF4}, {4,  ND5}, {8/3,NC5}, {8,  NA4},
        {4,  NA4}, {4,  NF4}, {8/3,NG4}, {8,  NF4}, {4,  NG4}, {4,  ND5},
        {8/3,NC5}, {8,  NA4}, {4,  NA4}, {4,  NC5}, {4/3,ND5}, {4,  NF5},
        {8/3,NC5}, {8,  NA4}, {4,  NA4}, {4,  NF4}, {8/3,NG4}, {8,  NF4}, 
        {8/3,NG4}, {16, NA4}, {16, NG4}, {8/3,NF4}, {8,  ND4}, {4,  ND4}, {4,NC4},
        {4/3,NF4}, {4,  NR}
    };

note sun[] =     // Ald Lang
    {
        {4,  ND5}, {16  ,NG4}, {16,  NA4}, {16,  NB4}, {16,  NC5}, {4  ,ND5}, {4,  NG4},
        {4  ,NE5}, {16,  NC5}, {16,  ND5}, {16,  NE5}, {16  ,NF5}, {4,  NG5}, {4,  NG4},
        {4,  NC5}, {16,  ND5}, {16,  NC5}, {16 , NB4}, {16,  NA4}, {4,  NB4}, 
                   {16,  NC5}, {16,  NB4}, {16,  NA4}, {16,  NG4}, {4,  NA4}, 
                   {16,  NG4}, {16,  NA4}, {16,  NB4}, {16,  NG4}, {4,  NB4},
        {4,  NR}
    };

#define TOTAL_MELODIES 3

note* melodies[TOTAL_MELODIES] = { happy, auld, sun };
int sizes[TOTAL_MELODIES] = {26, 45, 31};

int isPlayingMelody = 0;
int melodyNumber = 0;

int noteIx = 0;
int size = 0;
note* melody;

void start_melody(int m){
  if (isPlayingMelody == 0) {
      melodyNumber = m % TOTAL_MELODIES;
      melody = melodies[melodyNumber];
      size = sizes[melodyNumber];
      isPlayingMelody = 1;
      noteIx = 0;
  }
}

void stop_melody(){
    isPlayingMelody = 0;
}

note currentNote;
long duration = 0;

void play_note(int pin){
    static long wait_time = 0;
    if (isPlayingMelody == 0) return;


    if ((millis() - wait_time) >= duration) {
        wait_time = millis();
        currentNote = melody[noteIx];

        int tempo = currentNote.L;
        int freq = currentNote.n;
        duration = 0.7 * 4 * 1000 / tempo;

        noteIx ++;
        noTone(pin);
        tone(pin, freq);

        if (noteIx >= size) {
            stop_melody();
            noTone(pin);
            duration = 0;
        }
    }
    // delay(duration);

    
}
#endif
