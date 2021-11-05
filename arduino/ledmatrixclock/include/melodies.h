#ifndef __MELODY__H
#define __MELODY__H
#include "pitch.h"

typedef struct {
    int L, n;
} note;


void start_melody(int m);
void stop_melody();
void play_note(int);
#endif