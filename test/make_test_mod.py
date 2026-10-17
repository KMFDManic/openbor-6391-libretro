#!/usr/bin/env python3
"""Build a tiny synthetic OpenBOR mod used to smoke-test the libretro core.

Usage: make_test_mod.py <out_dir> [--width W --height H]

Creates <out_dir>/data/... (fonts, backgrounds, sounds, ogg music, menu and
level lists). Pack it afterwards with borpak:  borpak -b -d <out_dir> test.pak
Requires Pillow and ffmpeg (for the Vorbis music track).
"""
import argparse
import math
import os
import struct
import subprocess
import wave

from PIL import Image, ImageDraw, ImageFont


def write(path, text):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", newline="\r\n") as f:
        f.write(text)


def make_font(path, color):
    """16x16 grid of 8x8 glyphs, palette index 0 = transparent."""
    img = Image.new("P", (128, 128), 0)
    pal = [0, 0, 0] * 256
    pal[3:6] = color
    img.putpalette(pal)
    draw = ImageDraw.Draw(img)
    font = ImageFont.load_default()
    # OpenBOR takes every glyph's palette from glyph 0, so cell 0 must not be
    # empty (an empty cell is encoded without a palette).
    draw.rectangle([0, 0, 6, 7], fill=1)
    for code in range(32, 127):
        x, y = (code % 16) * 8, (code // 16) * 8
        draw.text((x, y - 2), chr(code), fill=1, font=font)
    img.save(path)


def make_background(path, w, h, label, hue):
    img = Image.new("RGB", (w, h))
    px = img.load()
    for y in range(h):
        for x in range(w):
            r = int(127 + 127 * math.sin((x + hue) / 23.0))
            g = int(127 + 127 * math.sin((y + hue) / 17.0))
            b = (x * 255 // max(1, w - 1)) ^ (y * 255 // max(1, h - 1))
            px[x, y] = (r, g, b)
    draw = ImageDraw.Draw(img)
    draw.rectangle([w // 2 - 70, h // 2 - 12, w // 2 + 70, h // 2 + 12], fill=(0, 0, 0))
    draw.text((w // 2 - 60, h // 2 - 6), label, fill=(255, 255, 255))
    # corner markers to check orientation / channel order (R, G, B, white)
    draw.rectangle([0, 0, 15, 15], fill=(255, 0, 0))
    draw.rectangle([w - 16, 0, w - 1, 15], fill=(0, 255, 0))
    draw.rectangle([0, h - 16, 15, h - 1], fill=(0, 0, 255))
    draw.rectangle([w - 16, h - 16, w - 1, h - 1], fill=(255, 255, 255))
    img.convert("P", palette=Image.ADAPTIVE, colors=256).save(path)


def make_wav(path, freq, ms, rate=22050):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    n = rate * ms // 1000
    with wave.open(path, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(rate)
        frames = b"".join(
            struct.pack("<h", int(12000 * math.sin(2 * math.pi * freq * i / rate)
                                  * (1 - i / n)))
            for i in range(n))
        w.writeframes(frames)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("--width", type=int, default=480)
    ap.add_argument("--height", type=int, default=272)
    a = ap.parse_args()
    d = os.path.join(a.out, "data")
    os.makedirs(os.path.join(d, "sprites"), exist_ok=True)
    os.makedirs(os.path.join(d, "bgs"), exist_ok=True)
    os.makedirs(os.path.join(d, "music"), exist_ok=True)

    write(os.path.join(d, "video.txt"), "video %dx%d\n" % (a.width, a.height))
    write(os.path.join(d, "models.txt"), "# no models in the smoke-test mod\n")
    write(os.path.join(d, "levels.txt"), "set Test\nz 160 232\n")

    for i, color in enumerate([(255, 255, 255), (255, 220, 0), (0, 220, 255), (255, 80, 80)]):
        name = "font.gif" if i == 0 else "font%d.gif" % (i + 1)
        make_font(os.path.join(d, "sprites", name), color)

    w, h = a.width, a.height
    make_background(os.path.join(d, "bgs", "logo.gif"), w, h, "LOGO SCREEN", 0)
    make_background(os.path.join(d, "bgs", "title.gif"), w, h, "TITLE SCREEN", 90)
    make_background(os.path.join(d, "bgs", "titleb.gif"), w, h, "MENU SCREEN", 180)
    make_background(os.path.join(d, "bgs", "loading.gif"), w, h, "LOADING", 270)

    make_wav(os.path.join(d, "sounds", "beep.wav"), 880, 80)
    make_wav(os.path.join(d, "sounds", "beep2.wav"), 1320, 120)
    make_wav(os.path.join(d, "sounds", "pause.wav"), 440, 200)

    # 4 s looping Vorbis track (220 Hz + 330 Hz) to exercise libvorbis
    subprocess.run(["ffmpeg", "-hide_banner", "-loglevel", "error", "-y",
                    "-f", "lavfi", "-i", "sine=frequency=220:duration=4",
                    "-f", "lavfi", "-i", "sine=frequency=330:duration=4",
                    "-filter_complex", "[0][1]amerge=inputs=2",
                    "-ac", "2", "-ar", "44100", "-c:a", "libvorbis", "-q:a", "3",
                    os.path.join(d, "music", "remix.ogg")], check=True)


if __name__ == "__main__":
    main()
