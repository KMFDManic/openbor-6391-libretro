/*
 * OpenBOR - http://www.chronocrash.com
 * -----------------------------------------------------------------------
 * Licensed under a BSD-style license, see LICENSE in OpenBOR root for details.
 *
 * libretro port - input
 *
 * Key codes: 1 + port * CONTROL_PAD_STRIDE + RETRO_DEVICE_ID_JOYPAD_xxx
 * (0 = unbound). RetroPad ids: B=0 Y=1 SELECT=2 START=3 UP=4 DOWN=5 LEFT=6
 * RIGHT=7 A=8 X=9 L=10 R=11 L2=12 R2=13 L3=14 R3=15.
 */

#ifndef CONTROL_H
#define CONTROL_H

#include <stdint.h>

#define CONTROL_MAX_PADS     4
#define CONTROL_PAD_STRIDE   16

#define RP_B       0
#define RP_Y       1
#define RP_SELECT  2
#define RP_START   3
#define RP_UP      4
#define RP_DOWN    5
#define RP_LEFT    6
#define RP_RIGHT   7
#define RP_A       8
#define RP_X       9
#define RP_L       10
#define RP_R       11
#define RP_L2      12
#define RP_R2      13
#define RP_L3      14
#define RP_R3      15

#define CONTROL_KEY(port, id) (1 + (port) * CONTROL_PAD_STRIDE + (id))

/* Select on the first pad opens/closes menus (OpenBOR "Esc") */
#define CONTROL_ESC CONTROL_KEY(0, RP_SELECT)

/* value used by the engine to clear a binding (outside every pad range) */
#define CONTROL_NONE (1 + (CONTROL_PAD_STRIDE * 99))

#define CONTROL_DEFAULTS(n, port) \
    enum { \
        CONTROL_DEFAULT##n##_UP         = CONTROL_KEY(port, RP_UP),    \
        CONTROL_DEFAULT##n##_RIGHT      = CONTROL_KEY(port, RP_RIGHT), \
        CONTROL_DEFAULT##n##_DOWN       = CONTROL_KEY(port, RP_DOWN),  \
        CONTROL_DEFAULT##n##_LEFT       = CONTROL_KEY(port, RP_LEFT),  \
        CONTROL_DEFAULT##n##_FIRE1      = CONTROL_KEY(port, RP_Y),     /* attack  */ \
        CONTROL_DEFAULT##n##_FIRE2      = CONTROL_KEY(port, RP_X),     /* attack2 */ \
        CONTROL_DEFAULT##n##_FIRE3      = CONTROL_KEY(port, RP_L),     /* attack3 */ \
        CONTROL_DEFAULT##n##_FIRE4      = CONTROL_KEY(port, RP_R),     /* attack4 */ \
        CONTROL_DEFAULT##n##_FIRE5      = CONTROL_KEY(port, RP_B),     /* jump    */ \
        CONTROL_DEFAULT##n##_FIRE6      = CONTROL_KEY(port, RP_A),     /* special */ \
        CONTROL_DEFAULT##n##_START      = CONTROL_KEY(port, RP_START), \
        CONTROL_DEFAULT##n##_SCREENSHOT = CONTROL_KEY(port, RP_R3)     \
    };

CONTROL_DEFAULTS(1, 0)
CONTROL_DEFAULTS(2, 1)
CONTROL_DEFAULTS(3, 2)
CONTROL_DEFAULTS(4, 3)

typedef struct
{
    int settings[32];
    unsigned int keyflags, newkeyflags;
    int kb_break;
}
s_playercontrols;

void control_exit();
void control_init(int joy_enable);
int control_usejoy(int enable);
int control_getjoyenabled();
int keyboard_getlastkey();
void control_setkey(s_playercontrols *pcontrols, unsigned int flag, int key);
int control_scankey();
char *control_getkeyname(unsigned int keycode);
void control_update(s_playercontrols **playercontrols, int numplayers);
void control_rumble(int port, int ratio, int msec);

/* libretro side */
void control_set_pad_state(int port, uint32_t buttons);
void control_tick_rumble(unsigned elapsed_ms);

#endif
