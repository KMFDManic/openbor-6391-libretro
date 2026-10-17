#!/usr/bin/env python3
"""Restore the standard "PACK" header of an OpenBOR .pak whose magic number
was altered so that stock engines refuse it (e.g. Streets of Rage X, whose
custom engine accepts 1D 11 14 7F instead of "PACK").

Only the 4-byte magic is rewritten; the file table and every file are
checked first, and nothing is written if the pak does not look like an
OpenBOR pak with plain-text contents.

    pak_fix_magic.py SORX.pak                 # writes SORX.pak.fixed
    pak_fix_magic.py SORX.pak -o out/SORX.pak
    pak_fix_magic.py SORX.pak --in-place
"""
import argparse
import os
import shutil
import struct
import sys

SIGNATURES = {
    "png": b"\x89PNG", "gif": b"GIF8", "ogg": b"OggS", "wav": b"RIFF",
    "webm": b"\x1aE\xdf\xa3", "bor": None,
}


def read_table(f, size):
    f.seek(size - 4)
    start = struct.unpack("<I", f.read(4))[0]
    if not 8 <= start < size - 4:
        raise ValueError("bad file table offset")
    f.seek(start)
    entries, pos = [], start
    while pos + 12 < size - 4:
        length, offset, length_data = struct.unpack("<III", f.read(12))
        if length <= 12 or length > 4096:
            raise ValueError("bad file table entry at %d" % pos)
        name = f.read(length - 12).split(b"\0")[0].decode("latin-1")
        if offset + length_data > start:
            raise ValueError("entry %s points outside the data area" % name)
        entries.append((name, offset, length_data))
        pos += length
    return entries


def check_contents(f, entries):
    bad = []
    for name, offset, length in entries:
        ext = name.rsplit(".", 1)[-1].lower()
        f.seek(offset)
        head = f.read(min(length, 512))
        sig = SIGNATURES.get(ext)
        if sig and not head.startswith(sig):
            bad.append(name)
        elif ext in ("txt", "c", "h") and head:
            binary = sum(1 for b in head if b < 9 or 13 < b < 32 and b != 26)
            if binary * 20 > len(head):
                bad.append(name)
    return bad


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("pak")
    ap.add_argument("-o", "--output")
    ap.add_argument("--in-place", action="store_true")
    a = ap.parse_args()

    size = os.path.getsize(a.pak)
    with open(a.pak, "rb") as f:
        header = f.read(8)
        if header[:4] == b"PACK":
            print("%s already has a standard PACK header" % a.pak)
            return 0
        if header[4:8] != b"\0\0\0\0":
            sys.exit("unexpected pak version field: %s" % header[4:8].hex())
        entries = read_table(f, size)
        bad = check_contents(f, entries)
    if bad:
        sys.exit("%d files do not look like plain data (e.g. %s): the contents are "
                 "encoded too, not only the header" % (len(bad), bad[0]))

    out = a.pak if a.in_place else (a.output or a.pak + ".fixed")
    if out != a.pak:
        shutil.copyfile(a.pak, out)
    with open(out, "r+b") as f:
        f.write(b"PACK")
    print("%s: %d files, magic %s -> PACK, written to %s" % (a.pak, len(entries), header[:4].hex(), out))
    return 0


if __name__ == "__main__":
    sys.exit(main())
