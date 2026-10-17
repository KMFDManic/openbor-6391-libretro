/*
 * OpenBOR - http://www.chronocrash.com
 * -----------------------------------------------------------------------
 * Licensed under a BSD-style license, see LICENSE in OpenBOR root for details.
 *
 * libretro port - video interface
 */

#ifndef VIDEO_H
#define VIDEO_H

#include "types.h"

void  video_init(void);
int   video_set_mode(s_videomodes);
int   video_copy_screen(s_screen *);
int   video_restore_mode(void);
int   video_current_refresh_rate(void);
void  video_stretch(int);
void  video_clearscreen(void);
void  video_set_color_correction(int, int);
void  video_exit(void);

#endif
