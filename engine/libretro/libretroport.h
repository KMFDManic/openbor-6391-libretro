/*
 * OpenBOR - http://www.chronocrash.com
 * -----------------------------------------------------------------------
 * Licensed under a BSD-style license, see LICENSE in OpenBOR root for details.
 *
 * libretro port - shared declarations between the port files.
 */

#ifndef LIBRETROPORT_H
#define LIBRETROPORT_H

#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>

#include "globals.h"

#ifndef stricmp
#define stricmp  strcasecmp
#endif
#ifndef strnicmp
#define strnicmp strncasecmp
#endif

/* Engine entry points expected from every port */
void borExit(int reset);
void openborMain(int argc, char **argv);

extern char packfile[MAX_FILENAME_LEN];
extern char paksDir[MAX_FILENAME_LEN];
extern char savesDir[MAX_FILENAME_LEN];
extern char logsDir[MAX_FILENAME_LEN];
extern char screenShotsDir[MAX_FILENAME_LEN];
extern char rootDir[MAX_FILENAME_LEN]; /* ends with a slash */

/* Builds "<rootDir><relPath>" into a static buffer (used for log files). */
char *libretro_root_path(const char *relPath);

/* ---- cooperative scheduling between retro_run() and the engine ---- */

/* Called by the engine side when a finished frame is ready (video_copy_screen). */
void libretro_yield_frame(void);

/* Called by the engine side when it is polling the clock without producing a
 * frame (busy-wait loops): hands control back to the frontend for one frame. */
void libretro_yield_idle(void);

/* Non-zero while the frontend is unloading the game: the engine must not yield. */
int libretro_unloading(void);

/* Called from borShutdown() with the formatted shutdown message. */
void libretro_shutdown_message(int status, const char *msg);

/* Wall-clock milliseconds (only for UI pacing such as the loading screen). */
unsigned libretro_realtime_ms(void);

/* Virtual engine clock in microseconds, advanced once per retro_run(). */
uint64_t libretro_virtual_usec(void);

/* Video mode change notification (from video.c) */
void libretro_video_mode_changed(int width, int height);

/* Rumble: strength 0..65535 on both motors for the given port */
void libretro_set_rumble(int port, uint16_t strength);

/* Logging to the frontend */
void libretro_log(int level, const char *fmt, ...);

/* Frame buffer produced by video.c (XRGB8888) */
extern uint32_t *libretro_framebuffer;
extern int libretro_fb_width;
extern int libretro_fb_height;

/* Audio settings requested by the engine (sblaster.c) */
extern int libretro_audio_started;
extern int libretro_audio_bits;
extern int libretro_audio_rate;

#endif
