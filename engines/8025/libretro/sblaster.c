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
    /* Build 8025 tries 24-bit/48 kHz first and falls back; the libretro
     * output is fixed at 16-bit/44.1 kHz, which is its last candidate. */
    if (bits != 16 || samplerate != 44100) return 0;
    libretro_audio_bits = bits;
    libretro_audio_rate = samplerate;
    libretro_audio_started = 1;
    return 1;
}

void SB_playstop(void)
{
    libretro_audio_started = 0;
}

#ifdef WEBM
/* The WebM decoder threads queue PCM into sound channels while the mixer
 * runs on the frontend thread (retro_run -> glue_mix_audio), so the audio
 * lock is a real (recursive) mutex in this build. */
#include "threads.h"

static bor_mutex *audio_mutex = NULL;

void SB_lock_audio_direct(void)
{
    if (!audio_mutex)
        audio_mutex = mutex_create();
    mutex_lock(audio_mutex);
}

void SB_unlock_audio_direct(void)
{
    mutex_unlock(audio_mutex);
}

void SB_lock_audio(void) { SB_lock_audio_direct(); }
void SB_unlock_audio(void) { SB_unlock_audio_direct(); }
#else
/* The mixer runs on the frontend thread while the engine is suspended, so
 * no locking is needed. */
void SB_lock_audio(void) {}
void SB_unlock_audio(void) {}
void SB_lock_audio_direct(void) {}
void SB_unlock_audio_direct(void) {}
#endif

void SB_setvolume(char dev, char volume)
{
    (void)dev;
    (void)volume;
}

void SB_updatevolume(int volume)
{
    (void)volume;
}
