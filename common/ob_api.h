/*
 * OpenBOR multi-engine libretro core - interface between the common libretro
 * layer (common/libretro.c) and one OpenBOR engine build (engines/<ver>/).
 *
 * Each engine version is compiled into a single relocatable object in which
 * every global symbol is made local except its ob_engine_<ver>() accessor,
 * so several engine versions can live in the same core without clashing.
 */

#ifndef OB_API_H
#define OB_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OB_API_VERSION 1

/* log levels (same values as enum retro_log_level) */
#define OB_LOG_DEBUG 0
#define OB_LOG_INFO  1
#define OB_LOG_WARN  2
#define OB_LOG_ERROR 3

/* Services the libretro layer offers to the engine (called on the engine
 * coroutine). */
typedef struct ob_host
{
    int api_version;

    /* a finished frame is ready in the engine framebuffer */
    void (*yield_frame)(void);
    /* the engine waits on its clock without drawing */
    void (*yield_idle)(void);
    /* non-zero while the frontend unloads the game (must not yield) */
    int (*unloading)(void);
    /* borShutdown() message (status != 0: error) */
    void (*shutdown_message)(int status, const char *msg);
    /* wall clock in ms (UI pacing only) */
    unsigned (*realtime_ms)(void);
    /* virtual engine clock in microseconds */
    uint64_t (*virtual_usec)(void);
    /* the engine framebuffer changed size */
    void (*video_mode_changed)(int width, int height);
    /* rumble, strength 0..65535 */
    void (*set_rumble)(int port, uint16_t strength);
    /* log a preformatted line */
    void (*log)(int level, const char *msg);
    /* borExit(): the engine has finished; never returns */
    void (*engine_exit)(int status);
} ob_host;

/* What an engine build provides. */
typedef struct ob_engine
{
    int api_version;
    const char *id;            /* "6391" */
    const char *name;          /* "v3.0 Build 6391" */

    void (*set_host)(const ob_host *host);

    /* Configure paths before main(): game pak, writable root (ends with
     * '/', contains Saves/ Logs/ ScreenShots/) and the pak's directory.
     * Returns 0 if a path does not fit the engine's buffers. */
    int (*set_paths)(const char *pak, const char *root_dir, const char *paks_dir);

    /* Engine entry point, runs on the engine coroutine; ends in
     * host->engine_exit(). */
    void (*main)(void);

    /* Non-zero once the engine finished loading the game (startup()). */
    int (*startup_done)(void);

    /* Called on the engine coroutine when it resumes while the host is
     * unloading: saves settings, frees everything, ends in engine_exit(). */
    void (*shutdown)(void);

    /* Input: RetroPad bitmask per port (bit = RETRO_DEVICE_ID_JOYPAD_*) */
    void (*set_pad)(int port, uint32_t buttons);
    void (*tick_rumble)(unsigned elapsed_ms);
    int (*analog_enabled)(void);

    /* Video: last converted frame, XRGB8888, pitch = width * 4 */
    const uint32_t *(*framebuffer)(int *width, int *height);
    void (*video_free)(void);

    /* Audio: mix `frames` stereo frames at 44.1 kHz into `out`.
     * Returns 0 (and leaves out untouched) when the engine has no audio. */
    int (*mix_audio)(int16_t *out, int frames);
} ob_engine;

const ob_engine *ob_engine_3711(void);
const ob_engine *ob_engine_4153(void);
const ob_engine *ob_engine_6391(void);
const ob_engine *ob_engine_7555(void);
const ob_engine *ob_engine_8025(void);
const ob_engine *ob_engine_8025_webm(void);

#ifdef __cplusplus
}
#endif

#endif
