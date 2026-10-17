/*
 * OpenBOR multi-engine libretro core - engine tag in .pak file names.
 *
 * A pak whose name ends with "[ob<engine>]" right before the extension is
 * always started with that engine, e.g.
 *
 *     SORX [ob8025_webm].pak     MFF4ever [ob7555].pak     SFFG[ob4153].pak
 *
 * The tag is case-insensitive; <engine> is one of the names offered by the
 * "Engine version" core option (3711, 4153, 6391, 7555, 8025, 8025_webm).
 * Header-only: used by the common layer and by the engine ports.
 */
#ifndef OB_PAK_TAG_H
#define OB_PAK_TAG_H

#include <stddef.h>
#include <string.h>
#include <ctype.h>

/* Returns a pointer to the file name part of `path`. */
static const char *ob_pak_basename(const char *path)
{
    const char *b = path, *p;
    for (p = path; *p; p++)
        if (*p == '/' || *p == '\\')
            b = p + 1;
    return b;
}

/* Finds the tag in a file name. On success returns 1, stores the engine name
 * (lower case) in `engine` and the length of the name without the tag and
 * the spaces before it in `stem_len`. */
static int ob_pak_find_tag(const char *path, char *engine, size_t engine_size, size_t *stem_len)
{
    const char *name = ob_pak_basename(path);
    const char *ext = strrchr(name, '.');
    const char *open, *close;
    size_t len, i;

    if (!ext || ext == name)
        ext = name + strlen(name);
    close = ext;
    while (close > name && close[-1] == ' ')
        close--;
    if (close == name || close[-1] != ']')
        return 0;
    close--;
    for (open = close; open > name && *open != '['; open--)
        ;
    if (*open != '[' || close - open < 4)
        return 0;
    if (tolower((unsigned char)open[1]) != 'o' || tolower((unsigned char)open[2]) != 'b')
        return 0;
    len = (size_t)(close - (open + 3));
    if (len == 0 || len >= engine_size)
        return 0;
    for (i = 0; i < len; i++)
        engine[i] = (char)tolower((unsigned char)open[3 + i]);
    engine[len] = 0;
    if (stem_len)
    {
        const char *s = open;
        while (s > name && s[-1] == ' ')
            s--;
        *stem_len = (size_t)(s - name);
    }
    return 1;
}

/* Writes the file name of `path` without the tag ("SORX [ob8025_webm].pak"
 * -> "SORX.pak"). Returns 0 if the name has no tag. */
static int ob_pak_untagged_name(const char *path, char *out, size_t out_size)
{
    char engine[32];
    size_t stem;
    const char *name = ob_pak_basename(path);
    const char *ext = strrchr(name, '.');

    if (!ob_pak_find_tag(path, engine, sizeof(engine), &stem))
        return 0;
    if (!ext || ext == name)
        ext = "";
    if (stem + strlen(ext) + 1 > out_size)
        return 0;
    memcpy(out, name, stem);
    strcpy(out + stem, ext);
    return 1;
}

#endif
