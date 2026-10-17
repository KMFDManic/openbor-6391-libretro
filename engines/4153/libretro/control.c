/*
 * OpenBOR - http://www.chronocrash.com
 * -----------------------------------------------------------------------
 * Licensed under a BSD-style license, see LICENSE in OpenBOR root for details.
 *
 * libretro port - input
 *
 * retro_run() polls the frontend and stores one RetroPad bitmask per port
 * with control_set_pad_state(); the engine reads it from control_update().
 */

#include <string.h>
#include "libretroport.h"
#include "control.h"

#define PAD_START 1
#define PAD_END   (CONTROL_PAD_STRIDE * CONTROL_MAX_PADS)

static int usejoy;
static uint32_t padstate[CONTROL_MAX_PADS];  /* latest state from retro_run() */
static uint32_t scanstate[CONTROL_MAX_PADS]; /* state latched by control_update() */
static uint32_t lastkey[CONTROL_MAX_PADS];
static int rumble_ms[CONTROL_MAX_PADS];

static const char *padnames[PAD_END + 1 + 1] = {
    "...",
#define CONTROLNAMES(x) \
    x" B",      \
    x" Y",      \
    x" Select", \
    x" Start",  \
    x" Up",     \
    x" Down",   \
    x" Left",   \
    x" Right",  \
    x" A",      \
    x" X",      \
    x" L",      \
    x" R",      \
    x" L2",     \
    x" R2",     \
    x" L3",     \
    x" R3",
    CONTROLNAMES("P1")
    CONTROLNAMES("P2")
    CONTROLNAMES("P3")
    CONTROLNAMES("P4")
    "undefined"
};

static int flag_to_index(unsigned int flag)
{
    int index = 0;
    unsigned int bit = 1;
    while (!((bit << index) & flag) && index < 31) ++index;
    return index;
}

static int lowest_bit(uint32_t v)
{
    int i;
    for (i = 0; i < 32; i++)
        if (v & (1u << i)) return i;
    return -1;
}

void control_set_pad_state(int port, uint32_t buttons)
{
    if (port < 0 || port >= CONTROL_MAX_PADS) return;
    padstate[port] = buttons;
}

void control_exit()
{
    int i;
    usejoy = 0;
    for (i = 0; i < CONTROL_MAX_PADS; i++)
    {
        if (rumble_ms[i]) libretro_set_rumble(i, 0);
        rumble_ms[i] = 0;
    }
}

void control_init(int joy_enable)
{
    usejoy = joy_enable;
}

int control_usejoy(int enable)
{
    usejoy = enable;
    return 0;
}

int control_getjoyenabled()
{
    return usejoy;
}

int keyboard_getlastkey(void)
{
    int i, ret = 0;
    for (i = 0; i < CONTROL_MAX_PADS; i++)
    {
        ret |= lastkey[i];
        lastkey[i] = 0;
    }
    return ret;
}

void control_setkey(s_playercontrols *pcontrols, unsigned int flag, int key)
{
    if (!pcontrols) return;
    pcontrols->settings[flag_to_index(flag)] = key;
    pcontrols->keyflags = pcontrols->newkeyflags = 0;
}

// Scan input for newly-pressed keys (used by the button remapping menu).
// Uses the state latched by the last control_update(), like the other ports:
// the engine calls this after update() has yielded, when padstate already
// holds the next frame and would be out of step with bothnewkeys.
// Return value:
// 0  = no key was pressed
// >0 = key code for pressed key
int control_scankey()
{
    static unsigned ready = 0;
    int i, k = 0;

    for (i = 0; i < CONTROL_MAX_PADS; i++)
    {
        if (scanstate[i])
        {
            k = CONTROL_KEY(i, lowest_bit(scanstate[i]));
            break;
        }
    }

    if (ready && k)
    {
        ready = 0;
        return k;
    }
    ready = (!k);
    return 0;
}

char *control_getkeyname(unsigned int keycode)
{
    if (keycode >= PAD_START && keycode <= PAD_END) return (char *)padnames[keycode];
    return "...";
}

void control_update(s_playercontrols **playercontrols, int numplayers)
{
    int player, i;

    for (i = 0; i < CONTROL_MAX_PADS; i++)
    {
        scanstate[i] = padstate[i];
        lastkey[i] = padstate[i];
    }

    for (player = 0; player < numplayers; player++)
    {
        s_playercontrols *pcontrols = playercontrols[player];
        unsigned int k = 0;

        for (i = 0; i < 32; i++)
        {
            int t = pcontrols->settings[i];
            if (t >= PAD_START && t <= PAD_END)
            {
                int portnum = (t - 1) / CONTROL_PAD_STRIDE;
                int shiftby = (t - 1) % CONTROL_PAD_STRIDE;
                if ((scanstate[portnum] >> shiftby) & 1) k |= (1u << i);
            }
        }
        pcontrols->kb_break = 0;
        pcontrols->newkeyflags = k & (~pcontrols->keyflags);
        pcontrols->keyflags = k;
    }
}

void control_rumble(int port, int msec)
{
    if (port < 0 || port >= CONTROL_MAX_PADS || msec <= 0) return;
    if (msec > rumble_ms[port]) rumble_ms[port] = msec;
    libretro_set_rumble(port, 0xC000);
}

void control_tick_rumble(unsigned elapsed_ms)
{
    int i;
    for (i = 0; i < CONTROL_MAX_PADS; i++)
    {
        if (!rumble_ms[i]) continue;
        if ((unsigned)rumble_ms[i] <= elapsed_ms)
        {
            rumble_ms[i] = 0;
            libretro_set_rumble(i, 0);
        }
        else
        {
            rumble_ms[i] -= elapsed_ms;
        }
    }
}
