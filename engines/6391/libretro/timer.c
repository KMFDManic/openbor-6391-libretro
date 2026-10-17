/*
 * OpenBOR - http://www.chronocrash.com
 * -----------------------------------------------------------------------
 * Licensed under a BSD-style license, see LICENSE in OpenBOR root for details.
 *
 * libretro port - timer
 *
 * The engine clock is virtual: it advances by exactly one video frame each
 * time retro_run() is called (see libretroport.c). Game speed therefore
 * follows the frontend (fast-forward, slow-motion and frame stepping work)
 * and is independent of host timing jitter.
 */

#include "timer.h"
#include "types.h"
#include "libretroport.h"

#define GETTIME_FREQ (1000)

static unsigned lastinterval = 0;
static unsigned newticks = 0;

void borTimerInit()
{
}

void borTimerExit()
{
}

unsigned timer_getinterval(unsigned freq)
{
    unsigned tickspassed, ebx, blocksize, now;

    for (;;)
    {
        now = timer_gettick() - newticks;
        ebx = now - lastinterval;
        blocksize = GETTIME_FREQ / freq;
        ebx += GETTIME_FREQ % freq;
        tickspassed = ebx / blocksize;
        if (tickspassed)
            break;
        /* The engine is waiting for time to pass without drawing anything
         * (e.g. a music-only fade): let the frontend run a frame. */
        libretro_yield_idle();
    }
    ebx -= ebx % blocksize;
    lastinterval += ebx;
    return tickspassed;
}

unsigned timer_gettick()
{
    return (unsigned)(libretro_virtual_usec() / 1000);
}

u64 timer_uticks()
{
    return libretro_virtual_usec();
}

unsigned get_last_interval()
{
    return lastinterval;
}

void set_last_interval(unsigned value)
{
    lastinterval = value;
}

void set_ticks(unsigned value)
{
    newticks = value;
}
