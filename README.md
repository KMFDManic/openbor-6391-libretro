# OpenBOR v6391 — libretro core

A [libretro](https://www.libretro.com/) core of the **OpenBOR** beat 'em up engine,
built from the official **v6391** release (`v3.0 Build 6391`,
[DCurrent/openbor@v6391](https://github.com/DCurrent/openbor/tree/v6391)).

Build 6391 is the version many existing mods were made for; this core lets you
run them in RetroArch (or any libretro frontend) with the frontend's input
mapping, shaders, recording, fast-forward and so on.

## Usage

Load a mod's `.pak` file as content.

The engine's writable data is kept under the frontend's **save directory**
(falling back to the system directory, then to the content directory):

```
<save dir>/OpenBOR/Saves/        settings (<mod>.cfg), high scores, saved games
<save dir>/OpenBOR/Logs/         OpenBorLog.txt, ScriptLog.txt
<save dir>/OpenBOR/ScreenShots/
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
Rumble is supported (enable it per player in the same menu).

## Building

Everything needed is bundled in `deps/` (zlib, libpng, libogg, libvorbis,
libretro-common/libco), so only a C compiler and `make` are required.

```sh
make                      # Linux/BSD -> openbor_libretro.so, macOS -> .dylib
make platform=win         # MinGW     -> openbor_libretro.dll
make DEBUG=1              # -O0 -g
make SYSTEM_ZLIB=1 SYSTEM_LIBPNG=1 SYSTEM_VORBIS=1   # use system libraries
```

`./build.sh` wraps the same commands (`./build.sh win` for a MinGW cross build,
`./build.sh test some.pak` for a build plus a headless smoke test).

## How the port works

OpenBOR is a blocking program: `openborMain()` never returns and presents
frames from deep inside its own loops (menus, scenes, fades, loading screens).
The port (`engine/libretro/`) runs the unmodified engine on a **libco
cooperative thread**:

* `video_copy_screen()` converts the engine's 32-bit screen to XRGB8888 and
  switches back to `retro_run()`, so each `retro_run()` produces exactly one
  engine frame. Everything runs on the frontend's thread — no locks, no
  audio thread.
* The engine clock is **virtual**: it advances by one 60 Hz frame per
  `retro_run()`, so game speed follows the frontend (fast-forward, slow-motion,
  frame advance) and is independent of host timing. Loops that wait on the
  clock without drawing (e.g. a music-only fade) hand a frame back to the
  frontend instead of spinning.
* Audio: `retro_run()` calls the engine mixer (`update_sample()`) for one
  frame's worth of 44.1 kHz stereo samples (735 per frame).
* The engine's first `video_set_mode()` happens during `retro_load_game()`,
  so the mod's real resolution (any `video WxH` in `data/video.txt`) is
  reported to the frontend; later changes use `SET_GEOMETRY` /
  `SET_SYSTEM_AV_INFO`.
* Closing content runs the engine's normal shutdown (settings saved, all
  resources freed); quitting from the engine menu shows the credits and then
  asks the frontend to close the core.

Changes to upstream engine files are kept minimal and guarded by
`#ifdef LIBRETRO` (`git diff` against the first commit shows all of them):
log/save paths, the loading-screen refresh clock, forcing 16-bit/44.1 kHz
output, the rumble entry in the control menu, and the shutdown/error hooks.

## Limitations

* No save states, no reset (OpenBOR's global state cannot be reinitialised in
  place); to load another game, close the content and load it again (the
  frontend reloads the core).
* WebM video cutscenes are not supported (they are skipped as on other ports
  built without WebM). Animated GIF scenes work.
* Gamma correction from the video menu is ignored (brightness works), as in
  the upstream software renderer.
* Paths (content and save directory) are limited to 255 characters by the
  engine.

## Testing without a frontend

`test/frontend.c` is a minimal headless libretro frontend (scripted pad input,
frame dumps to PPM, audio to WAV) and `test/make_test_mod.py` generates a tiny
synthetic mod (fonts, backgrounds, sounds, Vorbis music) that exercises the
logo, title, menus, options, button remapping, quit and unload paths.

```sh
python3 test/make_test_mod.py /tmp/mod      # needs Pillow and ffmpeg
(cd /tmp/mod && borpak -b -d . ../test.pak) # borpak: upstream tools/borpak
./build.sh test /tmp/test.pak
```

## License

OpenBOR is distributed under the BSD-style license in [LICENSE](LICENSE)
(the upstream readme is kept as [README-OpenBOR.md](README-OpenBOR.md)).
Bundled dependencies keep their own licenses (see each directory in `deps/`).
