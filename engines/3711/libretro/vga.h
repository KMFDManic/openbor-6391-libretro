/*
 * OpenBOR - http://www.chronocrash.com
 * -----------------------------------------------------------------------
 * Licensed under a BSD-style license, see LICENSE in OpenBOR root for details.
 *
 * libretro port - VGA-style interface expected by build 3711 (each port
 * provided its own vga.h at the time).
 */

#ifndef VGA_H
#define VGA_H

void vga_setpalette(unsigned char *palette);
void vga_set_color_correction(int gamma, int brightness);
void vga_vwait(void);

#endif
