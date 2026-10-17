/*
 * OpenBOR - http://www.chronocrash.com
 * -----------------------------------------------------------------------
 * Licensed under a BSD-style license, see LICENSE in OpenBOR root for details.
 *
 * libretro port - core entry points
 *
 * OpenBOR is written as a blocking program: openborMain() never returns and
 * presents frames from deep inside its own loops. To fit the libretro model
 * the engine runs on its own libco cooperative thread:
 *
 *   retro_run() --co_switch--> engine ... video_copy_screen()
 *                <--co_switch-- libretro_yield_frame()
 *
 * Exactly one engine frame is produced per retro_run(). Everything runs on
 * the frontend's thread, so no locking is needed anywhere (the mixer is
 * driven from retro_run() while the engine is suspended).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "libretro.h"
#include "libco.h"

#include "libretroport.h"
#include "packfile.h"
#include "video.h"
#include "control.h"
#include "soundmix.h"
#include "utils.h"
#include "ram.h"
#include "version.h"

/* ------------------------------------------------------------------------ */
/* Globals required by the engine                                           */
/* ------------------------------------------------------------------------ */

char packfile[MAX_FILENAME_LEN] = {"bor.pak"};
char paksDir[MAX_FILENAME_LEN];
char savesDir[MAX_FILENAME_LEN];
char logsDir[MAX_FILENAME_LEN];
char screenShotsDir[MAX_FILENAME_LEN];
char rootDir[MAX_FILENAME_LEN];

/* ------------------------------------------------------------------------ */
/* Frontend callbacks                                                       */
/* ------------------------------------------------------------------------ */

#define FPS              60.0
#define AUDIO_RATE       44100
#define ENGINE_STACK     (16 * 1024 * 1024)

static retro_environment_t        environ_cb;
static retro_video_refresh_t      video_cb;
static retro_audio_sample_t       audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_input_poll_t         input_poll_cb;
static retro_input_state_t        input_state_cb;
static retro_log_printf_t         log_cb;
static struct retro_rumble_interface rumble;
static int have_rumble;
static retro_perf_get_time_usec_t perf_get_time_usec;
static bool can_dupe;
static bool have_bitmasks;

/* ------------------------------------------------------------------------ */
/* Engine coroutine state                                                   */
/* ------------------------------------------------------------------------ */

enum { ENGINE_NONE, ENGINE_RUNNING, ENGINE_EXITED };

static cothread_t main_thread;
static cothread_t engine_thread;
static int engine_state = ENGINE_NONE;
static int engine_used = 0;          /* the engine's statics are single-use */
static int engine_exit_status = 0;
static int frame_ready = 0;
static int unloading = 0;
static int shutdown_requested = 0;   /* RETRO_ENVIRONMENT_SHUTDOWN sent */

static int mode_known = 0;           /* first video_set_mode() happened */
static int av_width = 320, av_height = 240;
static int av_max_width = 320, av_max_height = 240;

static uint64_t frame_count = 0;
static uint64_t virtual_usec = 0;

static int16_t audio_buffer[(AUDIO_RATE / 50 + 16) * 2];
static unsigned char mix_buffer[(AUDIO_RATE / 50 + 16) * 4];
static double audio_frac = 0.0;

static char last_error[512];

/* ------------------------------------------------------------------------ */
/* Helpers                                                                  */
/* ------------------------------------------------------------------------ */

void libretro_log(int level, const char *fmt, ...)
{
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (log_cb)
        log_cb((enum retro_log_level)level, "[OpenBOR] %s", buf);
    else
        fprintf(stderr, "[OpenBOR] %s", buf);
}

char *libretro_root_path(const char *relPath)
{
    static char filename[MAX_BUFFER_LEN];
    snprintf(filename, sizeof(filename), "%s%s", rootDir, relPath);
    return filename;
}

unsigned libretro_realtime_ms(void)
{
    if (perf_get_time_usec)
        return (unsigned)(perf_get_time_usec() / 1000);
#if defined(CLOCK_MONOTONIC)
    {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return (unsigned)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
    }
#else
    return (unsigned)(clock() * 1000 / CLOCKS_PER_SEC);
#endif
}

uint64_t libretro_virtual_usec(void)
{
    return virtual_usec;
}

int libretro_unloading(void)
{
    return unloading;
}

void libretro_set_rumble(int port, uint16_t strength)
{
    if (!have_rumble) return;
    rumble.set_rumble_state(port, RETRO_RUMBLE_STRONG, strength);
    rumble.set_rumble_state(port, RETRO_RUMBLE_WEAK, strength);
}

void libretro_shutdown_message(int status, const char *msg)
{
    if (status == 0 || !msg)
        return;
    snprintf(last_error, sizeof(last_error), "%s", msg);
    libretro_log(RETRO_LOG_ERROR, "%s\n", msg);
    if (environ_cb)
    {
        struct retro_message rmsg;
        rmsg.msg = last_error;
        rmsg.frames = 600;
        environ_cb(RETRO_ENVIRONMENT_SET_MESSAGE, &rmsg);
    }
}

static void advance_virtual_clock(void)
{
    frame_count++;
    virtual_usec = (uint64_t)(frame_count * 1000000.0 / FPS);
}

/* ------------------------------------------------------------------------ */
/* Cooperative scheduling                                                   */
/* ------------------------------------------------------------------------ */

void borShutdown(int status, char *msg, ...);

/* Called on the engine side right after control comes back from retro_run()
 * or retro_unload_game(). */
static void engine_resumed(void)
{
    if (unloading)
    {
        /* Never returns: frees the game and ends in borExit(). If a shutdown
         * is already in progress (e.g. quit credits), borShutdown() returns
         * immediately and the engine simply finishes without yielding. */
        borShutdown(0, "");
    }
}

void libretro_yield_frame(void)
{
    if (unloading || engine_state != ENGINE_RUNNING)
        return;
    frame_ready = 1;
    co_switch(main_thread);
    engine_resumed();
}

void libretro_yield_idle(void)
{
    if (unloading || engine_state != ENGINE_RUNNING)
    {
        /* cannot hand control back: let virtual time pass instead */
        advance_virtual_clock();
        return;
    }
    co_switch(main_thread);
    engine_resumed();
}

void libretro_video_mode_changed(int width, int height)
{
    av_width = width;
    av_height = height;

    if (!mode_known)
    {
        /* First mode set during retro_load_game(): report it from
         * retro_get_system_av_info() and pause the engine until retro_run(). */
        mode_known = 1;
        av_max_width = width;
        av_max_height = height;
        if (engine_state == ENGINE_RUNNING && !unloading)
        {
            co_switch(main_thread);
            engine_resumed();
        }
        return;
    }

    if (!environ_cb)
        return;

    if (width > av_max_width || height > av_max_height)
    {
        struct retro_system_av_info info;
        if (width > av_max_width) av_max_width = width;
        if (height > av_max_height) av_max_height = height;
        retro_get_system_av_info(&info);
        environ_cb(RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO, &info);
    }
    else
    {
        struct retro_game_geometry geom;
        geom.base_width = width;
        geom.base_height = height;
        geom.max_width = av_max_width;
        geom.max_height = av_max_height;
        geom.aspect_ratio = (float)width / (float)height;
        environ_cb(RETRO_ENVIRONMENT_SET_GEOMETRY, &geom);
    }
}

void borExit(int reset)
{
    engine_exit_status = reset;
    engine_state = ENGINE_EXITED;
    for (;;)
        co_switch(main_thread);
}

static void engine_entry(void)
{
    static char arg0[] = "openbor";
    char *argv[2] = { arg0, NULL };

    setSystemRam();
    packfile_mode(0);
    video_init();
    openborMain(1, argv);
    borExit(0);
}

/* ------------------------------------------------------------------------ */
/* libretro API                                                             */
/* ------------------------------------------------------------------------ */

RETRO_API void retro_set_environment(retro_environment_t cb)
{
    static const struct retro_controller_description port_desc[] = {
        { "RetroPad", RETRO_DEVICE_JOYPAD },
    };
    static const struct retro_controller_info ports[] = {
        { port_desc, 1 }, { port_desc, 1 }, { port_desc, 1 }, { port_desc, 1 },
        { NULL, 0 },
    };
    bool no_content = false;

    environ_cb = cb;
    cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_content);
    cb(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void *)ports);
}

RETRO_API void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }
RETRO_API void retro_set_audio_sample(retro_audio_sample_t cb) { audio_cb = cb; }
RETRO_API void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { audio_batch_cb = cb; }
RETRO_API void retro_set_input_poll(retro_input_poll_t cb) { input_poll_cb = cb; }
RETRO_API void retro_set_input_state(retro_input_state_t cb) { input_state_cb = cb; }

RETRO_API unsigned retro_api_version(void)
{
    return RETRO_API_VERSION;
}

RETRO_API void retro_init(void)
{
    struct retro_log_callback logging;
    struct retro_perf_callback perf;

    if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
        log_cb = logging.log;
    if (environ_cb(RETRO_ENVIRONMENT_GET_PERF_INTERFACE, &perf))
        perf_get_time_usec = perf.get_time_usec;
    if (!environ_cb(RETRO_ENVIRONMENT_GET_CAN_DUPE, &can_dupe))
        can_dupe = false;
    have_rumble = environ_cb(RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE, &rumble) ? 1 : 0;
    have_bitmasks = environ_cb(RETRO_ENVIRONMENT_GET_INPUT_BITMASKS, NULL);
}

RETRO_API void retro_deinit(void)
{
    log_cb = NULL;
    perf_get_time_usec = NULL;
    have_rumble = 0;
}

RETRO_API void retro_get_system_info(struct retro_system_info *info)
{
    memset(info, 0, sizeof(*info));
    info->library_name     = "OpenBOR";
#ifndef GIT_VERSION
#define GIT_VERSION ""
#endif
    info->library_version  = "v3.0 Build 6391" GIT_VERSION;
    info->valid_extensions = "pak";
    info->need_fullpath    = true;
    info->block_extract    = false;
}

RETRO_API void retro_get_system_av_info(struct retro_system_av_info *info)
{
    memset(info, 0, sizeof(*info));
    info->geometry.base_width   = av_width;
    info->geometry.base_height  = av_height;
    info->geometry.max_width    = av_max_width;
    info->geometry.max_height   = av_max_height;
    info->geometry.aspect_ratio = (float)av_width / (float)av_height;
    info->timing.fps            = FPS;
    info->timing.sample_rate    = AUDIO_RATE;
}

RETRO_API void retro_set_controller_port_device(unsigned port, unsigned device)
{
    (void)port;
    (void)device;
}

RETRO_API void retro_reset(void)
{
    /* OpenBOR keeps a lot of global state that cannot be reinitialised
     * without restarting the process; reset is not supported. */
    if (environ_cb)
    {
        struct retro_message rmsg = { "OpenBOR: reset is not supported, reload the content instead", 180 };
        environ_cb(RETRO_ENVIRONMENT_SET_MESSAGE, &rmsg);
    }
}

static void set_input_descriptors(void)
{
#define DESC(p) \
    { p, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,   "Left" }, \
    { p, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,     "Up" }, \
    { p, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,   "Down" }, \
    { p, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT,  "Right" }, \
    { p, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,      "Attack" }, \
    { p, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,      "Jump" }, \
    { p, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,      "Special" }, \
    { p, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,      "Attack 2" }, \
    { p, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,      "Attack 3" }, \
    { p, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,      "Attack 4" }, \
    { p, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,  "Start" }, \
    { p, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Menu / Back (Esc)" }, \
    { p, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R3,     "Screenshot" }, \
    { p, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_X, "Move (analog, if enabled)" }, \
    { p, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_Y, "Move (analog, if enabled)" },
    struct retro_input_descriptor desc[] = {
        DESC(0) DESC(1) DESC(2) DESC(3)
        { 0, 0, 0, 0, NULL }
    };
#undef DESC
    environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);
}

static void make_dir_path(char *dst, const char *base, const char *name)
{
    snprintf(dst, MAX_FILENAME_LEN, "%s%s", base, name);
}

static int setup_paths(const char *content_path)
{
    const char *base = NULL;
    char content_dir[MAX_FILENAME_LEN];
    char *slash;

    if (strlen(content_path) >= MAX_FILENAME_LEN)
    {
        libretro_log(RETRO_LOG_ERROR, "Content path too long (max %d chars): %s\n",
                     MAX_FILENAME_LEN - 1, content_path);
        return 0;
    }
    strcpy(packfile, content_path);

    snprintf(content_dir, sizeof(content_dir), "%s", content_path);
    slash = strrchr(content_dir, '/');
#ifdef _WIN32
    {
        char *bslash = strrchr(content_dir, '\\');
        if (bslash && (!slash || bslash > slash)) slash = bslash;
    }
#endif
    if (slash) *slash = 0;
    else strcpy(content_dir, ".");

    if (!environ_cb(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &base) || !base || !base[0])
        if (!environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &base) || !base || !base[0])
            base = content_dir;

    if (strlen(base) + 64 >= MAX_FILENAME_LEN)
    {
        libretro_log(RETRO_LOG_ERROR, "Save directory path too long: %s\n", base);
        return 0;
    }

    snprintf(rootDir, sizeof(rootDir), "%s/OpenBOR/", base);
    snprintf(paksDir, sizeof(paksDir), "%s", content_dir);
    make_dir_path(savesDir, rootDir, "Saves");
    make_dir_path(logsDir, rootDir, "Logs");
    make_dir_path(screenShotsDir, rootDir, "ScreenShots");

    dirExists(rootDir, 1);
    dirExists(savesDir, 1);
    dirExists(logsDir, 1);
    dirExists(screenShotsDir, 1);

    libretro_log(RETRO_LOG_INFO, "Game: %s\n", packfile);
    libretro_log(RETRO_LOG_INFO, "Data directory: %s\n", rootDir);
    return 1;
}

RETRO_API bool retro_load_game(const struct retro_game_info *game)
{
    enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;

    if (!game || !game->path)
        return false;

    if (engine_used)
    {
        struct retro_message rmsg = { "OpenBOR: restart the core to load another game", 300 };
        environ_cb(RETRO_ENVIRONMENT_SET_MESSAGE, &rmsg);
        libretro_log(RETRO_LOG_ERROR, "The engine can only run once per core load.\n");
        return false;
    }

    if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
    {
        libretro_log(RETRO_LOG_ERROR, "XRGB8888 is not supported by the frontend.\n");
        return false;
    }

    if (!setup_paths(game->path))
        return false;

    set_input_descriptors();

    main_thread = co_active();
    engine_thread = co_create(ENGINE_STACK, engine_entry);
    if (!engine_thread)
    {
        libretro_log(RETRO_LOG_ERROR, "Unable to create the engine thread.\n");
        return false;
    }

    engine_used = 1;
    engine_state = ENGINE_RUNNING;
    frame_count = 0;
    virtual_usec = 0;
    unloading = 0;
    shutdown_requested = 0;

    /* Run the engine until it has chosen its video mode, so that
     * retro_get_system_av_info() reports the real resolution. */
    co_switch(engine_thread);

    if (engine_state == ENGINE_EXITED)
    {
        libretro_log(RETRO_LOG_ERROR, "The engine stopped while loading%s%s\n",
                     last_error[0] ? ": " : ".", last_error);
        co_delete(engine_thread);
        engine_thread = NULL;
        return false;
    }

    return true;
}

RETRO_API bool retro_load_game_special(unsigned type, const struct retro_game_info *info, size_t num)
{
    (void)type; (void)info; (void)num;
    return false;
}

RETRO_API void retro_unload_game(void)
{
    if (engine_thread)
    {
        if (engine_state == ENGINE_RUNNING)
        {
            /* Let the engine save its settings and free everything. */
            unloading = 1;
            co_switch(engine_thread);
        }
        co_delete(engine_thread);
        engine_thread = NULL;
    }
    engine_state = ENGINE_EXITED;
    video_exit();
}

static void poll_input(void)
{
    int port;
    input_poll_cb();

    for (port = 0; port < CONTROL_MAX_PADS; port++)
    {
        uint32_t buttons = 0;
        unsigned id;

        if (have_bitmasks)
        {
            buttons = (uint16_t)input_state_cb(port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_MASK);
        }
        else
        {
            for (id = 0; id < 16; id++)
                if (input_state_cb(port, RETRO_DEVICE_JOYPAD, 0, id))
                    buttons |= 1u << id;
        }

        if (control_getjoyenabled())
        {
            const int threshold = 0x4000;
            int ax = input_state_cb(port, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_X);
            int ay = input_state_cb(port, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_Y);
            if (ax <= -threshold) buttons |= 1u << RP_LEFT;
            if (ax >=  threshold) buttons |= 1u << RP_RIGHT;
            if (ay <= -threshold) buttons |= 1u << RP_UP;
            if (ay >=  threshold) buttons |= 1u << RP_DOWN;
        }

        control_set_pad_state(port, buttons);
    }
}

static void output_audio(void)
{
    size_t frames, i;

    audio_frac += (double)AUDIO_RATE / FPS;
    frames = (size_t)audio_frac;
    audio_frac -= frames;
    if (frames > sizeof(audio_buffer) / (2 * sizeof(int16_t)))
        frames = sizeof(audio_buffer) / (2 * sizeof(int16_t));

    if (libretro_audio_started && engine_state == ENGINE_RUNNING)
    {
        if (libretro_audio_bits == 16)
        {
            update_sample((unsigned char *)audio_buffer, (int)(frames * 4));
        }
        else
        {
            update_sample(mix_buffer, (int)(frames * 2));
            for (i = 0; i < frames * 2; i++)
                audio_buffer[i] = (int16_t)(((int)mix_buffer[i] - 128) << 8);
        }
    }
    else
    {
        memset(audio_buffer, 0, frames * 2 * sizeof(int16_t));
    }

    audio_batch_cb(audio_buffer, frames);
}

RETRO_API void retro_run(void)
{
    if (engine_state == ENGINE_RUNNING)
    {
        unsigned before = (unsigned)(virtual_usec / 1000);

        poll_input();
        advance_virtual_clock();
        control_tick_rumble((unsigned)(virtual_usec / 1000) - before);

        frame_ready = 0;
        co_switch(engine_thread);
    }
    else
    {
        input_poll_cb();
        frame_ready = 0;
    }

    if (engine_state == ENGINE_EXITED && !shutdown_requested)
    {
        shutdown_requested = 1;
        libretro_log(RETRO_LOG_INFO, "Engine exited (status %d).\n", engine_exit_status);
        environ_cb(RETRO_ENVIRONMENT_SHUTDOWN, NULL);
    }

    if (frame_ready && libretro_framebuffer)
        video_cb(libretro_framebuffer, libretro_fb_width, libretro_fb_height,
                 libretro_fb_width * sizeof(uint32_t));
    else if (can_dupe)
        video_cb(NULL, libretro_fb_width ? libretro_fb_width : av_width,
                 libretro_fb_height ? libretro_fb_height : av_height, 0);
    else if (libretro_framebuffer)
        video_cb(libretro_framebuffer, libretro_fb_width, libretro_fb_height,
                 libretro_fb_width * sizeof(uint32_t));

    output_audio();
}

RETRO_API size_t retro_serialize_size(void) { return 0; }
RETRO_API bool retro_serialize(void *data, size_t size) { (void)data; (void)size; return false; }
RETRO_API bool retro_unserialize(const void *data, size_t size) { (void)data; (void)size; return false; }
RETRO_API void retro_cheat_reset(void) {}
RETRO_API void retro_cheat_set(unsigned index, bool enabled, const char *code) { (void)index; (void)enabled; (void)code; }
RETRO_API unsigned retro_get_region(void) { return RETRO_REGION_NTSC; }
RETRO_API void *retro_get_memory_data(unsigned id) { (void)id; return NULL; }
RETRO_API size_t retro_get_memory_size(unsigned id) { (void)id; return 0; }
