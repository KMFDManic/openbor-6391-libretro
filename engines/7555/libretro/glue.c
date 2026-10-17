/*
 * OpenBOR - http://www.chronocrash.com
 * -----------------------------------------------------------------------
 * Licensed under a BSD-style license, see LICENSE in OpenBOR root for details.
 *
 * libretro port - engine side of the multi-engine interface (ob_api.h).
 *
 * Forwards the port's libretro_* services to the host and exposes this
 * engine build as an ob_engine. Every global of this engine build except
 * OB_ENGINE_ACCESSOR is made local at link time.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "libretroport.h"
#include "openbor.h"     /* same macros as the engine (e.g. shutdown -> borShutdown in 3711) */
#include "packfile.h"
#include "video.h"
#include "control.h"
#include "soundmix.h"
#include "utils.h"
#include "ram.h"

#ifndef OB_ENGINE_ID
#error OB_ENGINE_ID must be defined (e.g. -DOB_ENGINE_ID=6391)
#endif
#define OB_STR2(x) #x
#define OB_STR(x) OB_STR2(x)
#define OB_CAT2(a, b) a##b
#define OB_CAT(a, b) OB_CAT2(a, b)
#define OB_ENGINE_ACCESSOR OB_CAT(ob_engine_, OB_ENGINE_ID)

/* ------------------------------------------------------------------------ */
/* Globals required by the engine                                           */
/* ------------------------------------------------------------------------ */

char packfile[MAX_FILENAME_LEN] = {"bor.pak"};
char paksDir[MAX_FILENAME_LEN];
char savesDir[MAX_FILENAME_LEN];
char logsDir[MAX_FILENAME_LEN];
char screenShotsDir[MAX_FILENAME_LEN];
char rootDir[MAX_FILENAME_LEN];

const ob_host *libretro_host = NULL;

void borShutdown(int status, char *msg, ...);

/* ------------------------------------------------------------------------ */
/* Services used by the port and the patched engine files                   */
/* ------------------------------------------------------------------------ */

char *libretro_root_path(const char *relPath)
{
    static char filename[MAX_BUFFER_LEN];
    snprintf(filename, sizeof(filename), "%s%s", rootDir, relPath);
    return filename;
}

void libretro_yield_frame(void)          { libretro_host->yield_frame(); }
void libretro_yield_idle(void)           { libretro_host->yield_idle(); }
int libretro_unloading(void)             { return libretro_host->unloading(); }
unsigned libretro_realtime_ms(void)      { return libretro_host->realtime_ms(); }
uint64_t libretro_virtual_usec(void)     { return libretro_host->virtual_usec(); }
void libretro_video_mode_changed(int w, int h) { libretro_host->video_mode_changed(w, h); }
void libretro_set_rumble(int port, uint16_t s) { libretro_host->set_rumble(port, s); }

void libretro_shutdown_message(int status, const char *msg)
{
    libretro_host->shutdown_message(status, msg);
}

void libretro_log(int level, const char *fmt, ...)
{
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    libretro_host->log(level, buf);
}

void borExit(int reset)
{
    libretro_host->engine_exit(reset);
    for (;;) ; /* not reached */
}

/* ------------------------------------------------------------------------ */
/* ob_engine implementation                                                 */
/* ------------------------------------------------------------------------ */

static void glue_set_host(const ob_host *host)
{
    libretro_host = host;
}

static int glue_set_paths(const char *pak, const char *root_dir, const char *paks_dir)
{
    if (strlen(pak) >= MAX_FILENAME_LEN || strlen(root_dir) + 16 >= MAX_FILENAME_LEN ||
        strlen(paks_dir) >= MAX_FILENAME_LEN)
        return 0;

    strcpy(packfile, pak);
    strcpy(rootDir, root_dir);
    strcpy(paksDir, paks_dir);
    snprintf(savesDir, sizeof(savesDir), "%sSaves", rootDir);
    snprintf(logsDir, sizeof(logsDir), "%sLogs", rootDir);
    snprintf(screenShotsDir, sizeof(screenShotsDir), "%sScreenShots", rootDir);

    dirExists(rootDir, 1);
    dirExists(savesDir, 1);
    dirExists(logsDir, 1);
    dirExists(screenShotsDir, 1);
    return 1;
}

static void glue_main(void)
{
    static char arg0[] = "openbor";
    char *argv[2] = { arg0, NULL };

    setSystemRam();
    packfile_mode(0);
    video_init();
    openborMain(1, argv);
    borExit(0);
}

extern int startup_done;
static int glue_startup_done(void)
{
    return startup_done;
}

static void glue_shutdown(void)
{
    /* Never returns unless a shutdown is already in progress (then the
     * engine finishes it without yielding). */
    borShutdown(0, "");
}

static int glue_analog_enabled(void)
{
    return control_getjoyenabled();
}

static const uint32_t *glue_framebuffer(int *width, int *height)
{
    *width = libretro_fb_width;
    *height = libretro_fb_height;
    return libretro_framebuffer;
}

static int glue_mix_audio(int16_t *out, int frames)
{
    static unsigned char tmp[4096 * 2];
    int i;

    if (!libretro_audio_started)
        return 0;

    if (libretro_audio_bits == 16)
    {
        update_sample((unsigned char *)out, frames * 4);
    }
    else
    {
        if (frames * 2 > (int)sizeof(tmp))
            frames = sizeof(tmp) / 2;
        update_sample(tmp, frames * 2);
        for (i = 0; i < frames * 2; i++)
            out[i] = (int16_t)(((int)tmp[i] - 128) << 8);
    }
    return 1;
}

static const ob_engine engine = {
    OB_API_VERSION,
    OB_STR(OB_ENGINE_ID),
    OB_ENGINE_NAME,
    glue_set_host,
    glue_set_paths,
    glue_main,
    glue_startup_done,
    glue_shutdown,
    control_set_pad_state,
    control_tick_rumble,
    glue_analog_enabled,
    glue_framebuffer,
    video_exit,
    glue_mix_audio,
};

const ob_engine *OB_ENGINE_ACCESSOR(void)
{
    return &engine;
}
