/*
 * OpenBOR - http://www.chronocrash.com
 * -----------------------------------------------------------------------
 * Licensed under a BSD-style license, see LICENSE in OpenBOR root for details.
 *
 * libretro port - video output
 *
 * The engine renders into a 32-bit s_screen whose pixels are 0x00BBGGRR
 * (see colour32() in pixelformat.c). Frames are converted here to the
 * libretro XRGB8888 format (0x00RRGGBB) and handed to retro_run() through
 * libretro_framebuffer, then the engine coroutine yields.
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "types.h"
#include "globals.h"
#include "video.h"
#include "vga.h"
#include "libretroport.h"

uint32_t *libretro_framebuffer = NULL;
int libretro_fb_width = 0;
int libretro_fb_height = 0;

static int mode_width = 0;
static int mode_height = 0;
static int brightness = 0;              /* -256..256, as in the SDL port */
static uint32_t palette32[256];         /* for 8-bit screens, XRGB8888 */

static int ensure_framebuffer(int width, int height)
{
    if (width <= 0 || height <= 0)
        return 0;

    if (libretro_framebuffer && width == libretro_fb_width && height == libretro_fb_height)
        return 1;

    free(libretro_framebuffer);
    libretro_framebuffer = (uint32_t *)calloc((size_t)width * height, sizeof(uint32_t));
    if (!libretro_framebuffer)
    {
        libretro_fb_width = libretro_fb_height = 0;
        return 0;
    }
    libretro_fb_width = width;
    libretro_fb_height = height;
    libretro_video_mode_changed(width, height);
    return 1;
}

void video_init(void)
{
    memset(palette32, 0, sizeof(palette32));
    brightness = 0;
}

void video_exit(void)
{
    free(libretro_framebuffer);
    libretro_framebuffer = NULL;
    libretro_fb_width = libretro_fb_height = 0;
    mode_width = mode_height = 0;
}

int video_set_mode(s_videomodes videomodes)
{
    if (videomodes.hRes == 0 && videomodes.vRes == 0)
    {
        /* term_videomodes(): keep the last buffer so the frontend can still
         * present something until the core is unloaded. */
        mode_width = mode_height = 0;
        return 1;
    }

    mode_width = videomodes.hRes;
    mode_height = videomodes.vRes;

    if (!ensure_framebuffer(mode_width, mode_height))
        return 0;

    memset(libretro_framebuffer, 0, (size_t)libretro_fb_width * libretro_fb_height * sizeof(uint32_t));
    return 1;
}

static inline uint32_t apply_brightness(uint32_t c)
{
    uint32_t r = (c >> 16) & 0xFF, g = (c >> 8) & 0xFF, b = c & 0xFF;

    if (brightness > 0)
    {
        /* blend towards white, alpha = brightness - 1 (SDL port semantics) */
        uint32_t a = (uint32_t)(brightness - 1);
        if (a > 255) a = 255;
        r += ((255 - r) * a) / 255;
        g += ((255 - g) * a) / 255;
        b += ((255 - b) * a) / 255;
    }
    else if (brightness < 0)
    {
        uint32_t a = (uint32_t)((-brightness) - 1);
        if (a > 255) a = 255;
        r = (r * (255 - a)) / 255;
        g = (g * (255 - a)) / 255;
        b = (b * (255 - a)) / 255;
    }
    return (r << 16) | (g << 8) | b;
}

static void convert_screen(const s_screen *src)
{
    int x, y;
    const int w = src->width, h = src->height;
    uint32_t *dst = libretro_framebuffer;

    switch (src->pixelformat)
    {
    case PIXEL_32:
    {
        const uint32_t *s = (const uint32_t *)src->data;
        for (y = 0; y < h; y++)
            for (x = 0; x < w; x++, s++, dst++)
            {
                uint32_t p = *s;
                *dst = ((p & 0xFF) << 16) | (p & 0xFF00) | ((p >> 16) & 0xFF);
            }
        break;
    }
    case PIXEL_16:
    {
        /* colour16(): BGR565, blue in the high bits */
        const uint16_t *s = (const uint16_t *)src->data;
        for (y = 0; y < h; y++)
            for (x = 0; x < w; x++, s++, dst++)
            {
                uint32_t p = *s;
                uint32_t r = (p & 0x1F) << 3;
                uint32_t g = ((p >> 5) & 0x3F) << 2;
                uint32_t b = ((p >> 11) & 0x1F) << 3;
                *dst = ((r | (r >> 5)) << 16) | ((g | (g >> 6)) << 8) | (b | (b >> 5));
            }
        break;
    }
    case PIXEL_8:
    case PIXEL_x8:
    default:
    {
        const unsigned char *s = src->data;
        for (y = 0; y < h; y++)
            for (x = 0; x < w; x++)
                *dst++ = palette32[*s++];
        break;
    }
    }

    if (brightness != 0)
    {
        size_t i, n = (size_t)w * h;
        for (i = 0; i < n; i++)
            libretro_framebuffer[i] = apply_brightness(libretro_framebuffer[i]);
    }
}

int video_copy_screen(s_screen *src)
{
    if (!src)
        return 0;

    if (ensure_framebuffer(src->width, src->height))
        convert_screen(src);

    libretro_yield_frame();
    return 1;
}

void video_clearscreen(void)
{
    if (libretro_framebuffer)
        memset(libretro_framebuffer, 0, (size_t)libretro_fb_width * libretro_fb_height * sizeof(uint32_t));
}

void video_set_color_correction(int gamma, int br)
{
    (void)gamma; /* gamma is only implemented by hardware backends upstream */
    if (br < -256) br = -256;
    if (br > 256) br = 256;
    brightness = br;
}

void vga_setpalette(unsigned char *pal)
{
    int i;
    for (i = 0; i < 256; i++)
        palette32[i] = ((uint32_t)pal[i * 3] << 16) | ((uint32_t)pal[i * 3 + 1] << 8) | pal[i * 3 + 2];
}

/* Build 8025: called to re-apply the current mode (e.g. after video
 * playback); the frame buffer is already sized by video_copy_screen(). */
int video_restore_mode(void)
{
    return 1;
}

/* Scaling and aspect are handled by the frontend. */
void video_stretch(int enable)
{
    (void)enable;
}

/* The core always runs at 60 Hz. */
int video_current_refresh_rate(void)
{
    return 60;
}

/* Frame pacing is done by the frontend. */
void vga_vwait(void)
{
}
