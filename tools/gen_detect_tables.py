#!/usr/bin/env python3
"""Generate common/detect_tables.h from the engine sources.

For every engine in engines/<id>/ this extracts the vocabulary the engine
understands:
  * models.txt commands, model (character) file commands, level file
    commands, levels.txt commands     (createXxxCommandList in commands.c)
  * script builtin function names     (Script_LoadSystemFunctions)
  * script constants for openborconstant()   (ICMPCONST & co.)

Run it again after adding or updating an engine:
    python3 tools/gen_detect_tables.py
"""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ENGINES = os.path.join(ROOT, "engines")

LIST_FUNCS = {
    "modelstxt":  "createModelstxtCommandList",
    "model":      "createModelCommandList",
    "level":      "createLevelCommandList",
    "levelorder": "createLevelOrderCommandList",
}


def read(path):
    with open(path, encoding="latin-1") as f:
        return f.read()


def find_file(engine_dir, name, must_contain):
    for dirpath, _, files in os.walk(engine_dir):
        if "libretro" in dirpath.split(os.sep):
            continue
        for fn in files:
            if fn == name:
                p = os.path.join(dirpath, fn)
                if must_contain in read(p):
                    return p
    raise SystemExit("%s: no %s containing %s" % (engine_dir, name, must_contain))


def function_body(src, name):
    m = re.search(r"\b%s\s*\([^)]*\)\s*\{" % re.escape(name), src)
    if not m:
        return None
    i, depth = m.end(), 1
    while depth and i < len(src):
        if src[i] == "{":
            depth += 1
        elif src[i] == "}":
            depth -= 1
        i += 1
    return src[m.end():i]


def strip_comments(src):
    src = re.sub(r"/\*.*?\*/", "", src, flags=re.S)
    return re.sub(r"//[^\n]*", "", src)


def command_lists(engine_dir):
    path = find_file(engine_dir, "commands.c", "createModelCommandList")
    src = strip_comments(read(path))
    out = {}
    for cat, fn in LIST_FUNCS.items():
        body = function_body(src, fn) or ""
        words = set(w.lower() for w in re.findall(r'"([A-Za-z0-9_]+)"\s*\)', body))
        # dynamic entries such as sprintf(buf, "attack%d", i)
        for fmt in re.findall(r'sprintf\s*\(\s*\w+\s*,\s*"([A-Za-z_]+)%[di]"', body):
            words.add(fmt.lower() + "#")
        out[cat] = words
    return out


def builtins(engine_dir):
    names = set()
    for dirpath, _, files in os.walk(engine_dir):
        if "libretro" in dirpath.split(os.sep):
            continue
        for fn in files:
            if fn.endswith(".c"):
                src = read(os.path.join(dirpath, fn))
                if "Script_LoadSystemFunctions" not in src:
                    continue
                body = function_body(strip_comments(src), "Script_LoadSystemFunctions") or ""
                names.update(re.findall(r'\(\s*void\s*\*\s*\)\s*\w+\s*,\s*"(\w+)"', body))
    if not names:
        raise SystemExit("%s: no builtins found" % engine_dir)
    return names


def constants(engine_dir):
    names = set()
    for dirpath, _, files in os.walk(engine_dir):
        if "libretro" in dirpath.split(os.sep):
            continue
        for fn in files:
            if not fn.endswith(".c"):
                continue
            src = strip_comments(read(os.path.join(dirpath, fn)))
            if "ICMPCONST" not in src:
                continue
            names.update(re.findall(r"\bICMPCONST\s*\(\s*(\w+)\s*\)", src))
            for prefix in re.findall(r"\bICMPSCONST[ABC]\s*\(\s*(\w+)", src):
                names.add(prefix + "#")
            # build 3711: strnicmp(constname, "ANI_ATTACK", 10) style prefixes
            for prefix in re.findall(r'strnicmp\s*\(\s*constname\s*,\s*"(\w+)"', src):
                names.add(prefix + "#")
    names.discard("x")
    return set(n.upper() for n in names)


def properties(engine_dir):
    """String tables used by the script property/variant functions
    (svlist, eplist*, proplist*, typelist, ...)."""
    names = set()
    for dirpath, _, files in os.walk(engine_dir):
        if "libretro" in dirpath.split(os.sep):
            continue
        for fn in files:
            if not fn.endswith(".c"):
                continue
            path = os.path.join(dirpath, fn)
            if "openborscript" not in path:
                continue
            src = strip_comments(read(path))
            for m in re.finditer(r"static\s+const\s+char\s*\*\s*\w+\[\]\s*=\s*\{(.*?)\};", src, re.S):
                names.update(w.lower() for w in re.findall(r'"([A-Za-z0-9_.]+)"', m.group(1)))
    return names


def c_array(name, words):
    words = sorted(words)
    lines = ["static const char *const %s[] = {" % name]
    row = "   "
    for w in words:
        item = ' "%s",' % w
        if len(row) + len(item) > 78:
            lines.append(row)
            row = "   "
        row += item
    lines.append(row)
    lines.append("    NULL\n};")
    return "\n".join(lines)


def main():
    ids = sorted(int(d) for d in os.listdir(ENGINES) if d.isdigit())
    parts = ["/* Generated by tools/gen_detect_tables.py - do not edit. */",
             "#ifndef OB_DETECT_TABLES_H", "#define OB_DETECT_TABLES_H", "",
             "/* Entries ending in '#' match the prefix followed by digits. */", ""]
    entries = []
    for eid in ids:
        d = os.path.join(ENGINES, str(eid))
        lists = command_lists(d)
        b = builtins(d)
        c = constants(d)
        pr = properties(d)
        for cat in LIST_FUNCS:
            parts.append(c_array("tok_%s_%d" % (cat, eid), lists[cat]))
        parts.append(c_array("tok_builtin_%d" % eid, b))
        parts.append(c_array("tok_const_%d" % eid, c))
        parts.append(c_array("tok_prop_%d" % eid, pr))
        parts.append("")
        entries.append(eid)
        sys.stderr.write("%d: %s builtins=%d constants=%d" % (
            eid, " ".join("%s=%d" % (k, len(v)) for k, v in lists.items()), len(b), len(c)) + " properties=%d\n" % len(pr))

    parts.append("typedef struct {\n    int id;\n    const char *const *modelstxt, *const *model, *const *level, *const *levelorder,\n"
                 "                      *const *builtin, *const *constant, *const *property;\n} ob_vocab;\n")
    parts.append("static const ob_vocab ob_vocabs[] = {")
    for eid in entries:
        parts.append("    { %d, tok_modelstxt_%d, tok_model_%d, tok_level_%d, tok_levelorder_%d, tok_builtin_%d, tok_const_%d, tok_prop_%d },"
                     % ((eid,) * 8))
    parts.append("};")
    parts.append("#define OB_NUM_VOCABS %d" % len(entries))
    parts.append("\n#endif")
    with open(os.path.join(ROOT, "common", "detect_tables.h"), "w") as f:
        f.write("\n".join(parts) + "\n")


if __name__ == "__main__":
    main()
