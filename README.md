# OpenBOR multi-engine libretro core

A [libretro](https://www.libretro.com/) core of the **OpenBOR** beat 'em up
engine that contains **several official OpenBOR releases** and picks the
right one for each game:

| Engine | Upstream source | Typical mods |
|---|---|---|
| **v3.0 Build 3711** (2012) | [DCurrent/openbor@6ec17fa4](https://github.com/DCurrent/openbor/commit/6ec17fa411b237f789c910e1234a33a618ab236d) | classic 3.x / Beats of Rage era mods |
| **v3.0 Build 6391** (2018) | [DCurrent/openbor tag v6391](https://github.com/DCurrent/openbor/tree/v6391) | the long-lived "stable" most mods were built with |
| **v4.0 Build 7555** (2024) | [DCurrent/openbor@b1676f97](https://github.com/DCurrent/openbor/commit/b1676f97) | recent mods, e.g. *Mighty Final Fight Forever* |

OpenBOR versions are not fully compatible with each other: newer mods use
commands, script functions and constants that older engines reject, and older
mods can behave differently on newer engines. That is why players keep
several OpenBOR executables around; this core does the choosing for them.

## Usage

Load a mod's `.pak` file as content.

### How the engine is chosen

1. **Core option** `Engine version` (`auto`, `7555`, `6391`, `3711`). It can be
   saved per game with RetroArch's per-game core options. A forced version
   is used as is.
2. **Auto-detection** (default). The `.pak` is scanned before starting: the
   commands of `models.txt`, `levels.txt` and of every model and level file
   they reference, plus the scripts' builtin functions, `openborconstant()`
   names and quoted property/variant names, and the `video WxH` syntax. Each
   engine's vocabulary is extracted from its own sources
   (`tools/gen_detect_tables.py` → `common/detect_tables.h`). The engine
   with the fewest unknown items wins; on a tie the **oldest** one is chosen,
   as it is the closest to what the mod was written for.
3. **Fallback.** The chosen engine loads the whole game (models, levels,
   scripts) inside `retro_load_game()`. If it stops with an error — an
   unknown command, an invalid setting — the next engine is tried (newer
   ones first, then older ones), and an on-screen message says which engine
   is in use.

The frontend log shows the decision, e.g.
`Auto-detected engine 6391 (10 items need a newer engine than 3711, e.g. ...)`.

### Data files

Each engine keeps its writable data under the frontend's **save directory**
(falling back to the system directory, then to the content directory), in a
folder per engine version because settings, high scores and saved games are
raw structures whose layout changes between versions:

```
<save dir>/OpenBOR/<engine>/Saves/        settings (<mod>.cfg), high scores, saved games
<save dir>/OpenBOR/<engine>/Logs/         OpenBorLog.txt, ScriptLog.txt
<save dir>/OpenBOR/<engine>/ScreenShots/
```

Check `Logs/OpenBorLog.txt` first when a mod does not start; engine errors are
also shown as an on-screen message.

### Default controls (RetroPad, up to 4 players)

| RetroPad | OpenBOR |
|---|---|
| D-Pad | Move |
| Y | Attack 1 |
| B | Jump |
| A | Special |
| X | Attack 2 |
| L / R | Attack 3 / Attack 4 |
| Start | Start / pause |
| Select (player 1) | Esc (back / exit menus) |
| R3 | Screenshot |
| Left stick | Move, when "GamePads/Analog" is enabled in Control Options |

Buttons can also be remapped from the engine's own *Options → Control Options*
menu (names are shown as `P1 X`, `P2 Start`, …), or from the frontend.
Rumble is supported.

## Building

Everything needed is bundled in `deps/` (zlib, libpng, libogg, libvorbis,
libretro-common/libco), so only a C compiler, `make` and binutils (`ld -r`,
`objcopy`) are required.

```sh
make                          # Linux/BSD -> openbor_libretro.so, macOS -> .dylib
make platform=win             # MinGW     -> openbor_libretro.dll
make ENGINES="6391 7555"      # choose the engine versions to include
make DEBUG=1                  # -O0 -g
make SYSTEM_ZLIB=1 SYSTEM_LIBPNG=1 SYSTEM_VORBIS=1   # use system libraries
```

`./build.sh` wraps the same commands (`./build.sh win` for a MinGW cross build,
`./build.sh test some.pak` for a build plus a headless smoke test).

## How it works

### Several engines in one binary

Every engine version lives in `engines/<build>/` (the unmodified upstream
`engine/` directory, imported in its own commit, plus a `libretro/` port
layer). Each one is compiled with its own include paths and flags, partially
linked into a single object (`ld -r`), and then every global symbol except its
`ob_engine_<build>()` accessor is made local (`objcopy --keep-global-symbol`,
`ld -exported_symbol` on macOS). The thousands of identically named globals
of the different versions therefore never clash, and each engine keeps its
own state — which is also what makes the in-process fallback possible.

The version-independent libretro layer (`common/libretro.c`) talks to the
engines through a small interface (`common/ob_api.h`).

### Running a blocking engine under libretro

OpenBOR is a blocking program: `openborMain()` never returns and presents
frames from deep inside its own loops (menus, scenes, fades, loading screens).
The engine runs on a **libco cooperative thread**:

* `video_copy_screen()` converts the engine's screen (8-bit paletted, 16 or
  32-bit, depending on the version and mod) to XRGB8888 and switches back to
  `retro_run()`, so each `retro_run()` produces exactly one engine frame.
  Everything runs on the frontend's thread — no locks, no audio thread.
* The engine clock is **virtual**: it advances by one 60 Hz frame per
  `retro_run()`, so game speed follows the frontend (fast-forward,
  slow-motion, frame advance). Loops that wait on the clock without drawing
  hand a frame back to the frontend instead of spinning.
* Audio: `retro_run()` calls the engine mixer for one frame's worth of
  44.1 kHz stereo samples (735 per frame).
* The mod's real resolution is reported from `retro_get_system_av_info()`;
  later changes use `SET_GEOMETRY` / `SET_SYSTEM_AV_INFO`.
* Closing content runs the engine's normal shutdown (settings saved, all
  resources freed); quitting from the engine menu asks the frontend to close
  the core.

Changes to upstream engine files are kept minimal and guarded by
`#ifdef LIBRETRO` (`git diff` against each engine's import commit shows all of
them): log/save paths, the loading-screen refresh clock, forced 16-bit/44.1 kHz
output where the engine still allowed other rates, the rumble entry in the
control menu, the shutdown/error hooks, and — for build 3711 — larger save
path buffers (they were 128 bytes) and `exit()` replaced by the port's exit.
Two upstream memory bugs found with valgrind are fixed the same way: build
6391 allocated the per-frame collision box lists for one entry but filled
`max_collisons` (2) of them, and build 3711 iterated one element past the
damage-over-time arrays.

## Limitations

* No save states and no reset (OpenBOR's global state cannot be reinitialised
  in place). Each engine can be started once per core load; to load another
  game, close the content (RetroArch reloads the core).
* WebM video cutscenes are not supported (skipped, as on upstream builds
  without WebM). Animated GIF scenes work.
* Gamma correction from the video menu is ignored (brightness works), as in
  the upstream software renderers.
* Engine 3711 only accepts the numeric `video N` modes of its time.
* When an engine fails while loading and the next one is tried, the failed
  engine cannot free what it had loaded (upstream only frees after a
  complete startup): that memory (typically a few MB to tens of MB) stays
  allocated until the content is closed.
* Auto-detection reads vocabulary, not behaviour: a mod that loads on an
  older engine but relies on a newer engine's behaviour may need the
  `Engine version` option.

## Testing without a frontend

* `test/frontend.c` — minimal headless libretro frontend (scripted pad input,
  frame dumps to PPM, audio to WAV; core options from environment variables,
  e.g. `OPENBOR_ENGINE=3711`).
* `test/make_test_mod.py` — generates a tiny synthetic mod (fonts,
  backgrounds, sounds, Vorbis music; `--video-mode N` for 3.x engines).
* `test/detect_cli.c` — prints the engine auto-detection would pick
  (`OB_DETECT_DEBUG=1` lists every item that differs between engines).

```sh
python3 test/make_test_mod.py /tmp/mod      # needs Pillow and ffmpeg
(cd /tmp/mod && borpak -b -d . ../test.pak) # borpak: upstream tools/borpak
./build.sh test /tmp/test.pak
```

## License

OpenBOR is distributed under the BSD-style license in [LICENSE](LICENSE)
(the upstream readme is kept as [README-OpenBOR.md](README-OpenBOR.md)).
Bundled dependencies keep their own licenses (see each directory in `deps/`).
