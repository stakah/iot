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

note sun[] =     // Sun
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

#define PLAYER_IDLE 0
#define PLAYER_BEGIN_MUSIC 1
#define PLAYER_STOP_MUSIC 2
#define PLAYER_PLAYING 3

int playerState = PLAYER_IDLE;

int melodyNumber = 0;

int noteIx = 0;
int size = 0;
note* melody;

void start_melody(int m){
  switch (playerState) {
      case PLAYER_IDLE:
        return;
        break;
      case PLAYER_BEGIN_MUSIC:
        melodyNumber = m % TOTAL_MELODIES;
        melody = melodies[melodyNumber];
        size = sizes[melodyNumber];
        playerState = PLAYER_PLAYING;
        noteIx = 0;
        break;
      case PLAYER_STOP_MUSIC:
        playerState = PLAYER_IDLE;
        break;
      case PLAYER_PLAYING:
        break;
  }
}

void toggle_player(){
    switch(playerState) {
        case PLAYER_PLAYING:
          stop_melody();
          break;
        case PLAYER_IDLE:
          start_melody(melodyNumber);
          break;
    }
}
void begin_player(){
    playerState = PLAYER_BEGIN_MUSIC;
}

void stop_melody(){
    playerState = PLAYER_STOP_MUSIC;
}

note currentNote;
long duration = 0;

void play_note(int pin){
    static long wait_time = 0;

    if (playerState != PLAYER_PLAYING) {
        // noTone(pin);
        return;
    }

    long mill = millis();
    if ((mill - wait_time) >= duration) {
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
