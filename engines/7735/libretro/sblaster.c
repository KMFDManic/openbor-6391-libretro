/*
 * OpenBOR - http://www.chronocrash.com
 * -----------------------------------------------------------------------
 * Licensed under a BSD-style license, see LICENSE in OpenBOR root for details.
 *
 * libretro port - audio output
 *
 * No audio thread: retro_run() calls update_sample() itself for each
 * frame's worth of samples while the engine coroutine is suspended, so the
 * mixer never runs concurrently with the engine.
 */

#include "sblaster.h"
#include "soundmix.h"
#include "libretroport.h"

int libretro_audio_started = 0;
int libretro_audio_bits = 16;
int libretro_audio_rate = 44100;

int SB_playstart(int bits, int samplerate)
{
    if (bits != 8 && bits != 16) return 0;
    libretro_audio_bits = bits;
    libretro_audio_rate = samplerate;
    libretro_audio_started = 1;
    return 1;
}

void SB_playstop(void)
{
    libretro_audio_started = 0;
}

void SB_setvolume(char dev, char volume)
{
    (void)dev;
    (void)volume;
}

void SB_updatevolume(int volume)
{
    (void)volume;
}
