#ifndef SOUND_H
#define SOUND_H

#define SOUND_SAMPLE_RATE 16000

void init_sound();
void sound_irq();

void shot_play();
void theme_play();

#endif
