#!/usr/bin/env python3

import json
import os

COMP_NAMES = {
    "lc" : None,
    "rc" : None,
    "ec" : None,
    "rs" : None,
    "no" : "file_normal",
    "fi" : "file",
    "di" : "file_dir",
    "ln" : "file_link",
    "pi" : "file_fifo",
    "so" : "file_sock",
    "bd" : "file_block",
    "cd" : "file_char",
    "mi" : "file_missing",
    "or" : "file_orphan",
    "ex" : "file_exec",
    "do" : "file_door",
    "su" : "file_setuid",
    "sg" : "file_setgid",
    "st" : "file_sticky",
    "ow" : "file_other_writable",
    "tw" : "file_sticky_other_writable",
    "ca" : "file_capability",
    "mh" : "file_multi_hard_link",
    "cl" : None
}

LS_COLORS = os.environ["LS_COLORS"]

LS_COLORS = LS_COLORS.split(":")

# Extract x=y pairs
LS_COLORS = [i.split("=") for i in LS_COLORS]

out = {}

def parse_out(cseq, out):
    trim = 0
    if cseq[0] == 38:
        if cseq[1] == 5:
            out["fg"] = int(cseq[2])
            trim = 3
        elif cseq[1] == 2:
            out["fg"] = [int(cseq[2]),int(cseq[3]),int(cseq[4])]
            trim = 5
        else:
            trim = 1
    elif cseq[0] == 48:
        if cseq[1] == 5:
            out["bg"] = int(cseq[2])
            trim = 3
        elif cseq[1] == 2:
            out["bg"] = list(int(cseq[2]),int(cseq[3]),int(cseq[4]))
            trim = 5
        else:
            trim = 1
    elif cseq[0] >= 30 and cseq[0] < 38:
        out["fg"] = (cseq[0] - 30)
        trim = 1
    elif cseq[0] >= 40 and cseq[0] < 48:
        out["bg"] = (cseq[0] - 30)
        trim = 1
    elif cseq[0] == 39:
        out["fg"] = None;
        trim = 1
    elif cseq[0] == 49:
        out["bg"] = None;
        trim = 1
    elif cseq[0] >= 90 and cseq[0] < 98:
        out["fg"] = (cseq[0] - 90) + 8
        trim = 1
    elif cseq[0] >= 100 and cseq[0] < 108:
        out["bg"] = (cseq[0] - 100) + 8
        trim = 1
    elif cseq[0] == 0:
        trim = 1
    elif cseq[0] == 1:
        out["bold"] = True
        trim = 1
    elif cseq[0] == 2:
        out["faint"] = True
        trim = 1
    elif cseq[0] == 3:
        out["italic"] = True
        trim = 1
    elif cseq[0] == 4:
        out["underline"] = True
        trim = 1
    elif cseq[0] == 5:
        out["blink"] = True
        trim = 1
    elif cseq[0] == 6:
        out["fast_blink"] = True
        trim = 1
    elif cseq[0] == 7:
        out["reverse"] = True
        trim = 1
    elif cseq[0] == 8:
        trim = 1
    elif cseq[0] == 9:
        out["strikethrough"] = True
        trim = 1
    elif cseq[0] == 21:
        out["double_underline"] = True
        trim = 1
    elif cseq[0] == 22:
        out["bold"] = False
        out["faint"] = False
        trim = 1
    elif cseq[0] == 23:
        out["italic"] = False
        trim = 1
    elif cseq[0] == 24:
        out["underline"] = False
        out["double_underline"] = False
        trim = 1
    elif cseq[0] == 25:
        out["blink"] = False
        out["fast_blink"] = False
        trim = 1
    elif cseq[0] == 27:
        out["reverse"] = False
        trim = 1
    elif cseq[0] == 28:
        trim = 1
    elif cseq[0] == 29:
        out["strikethrough"] = False
        trim = 1
    return (cseq[trim:], out)

def make_type_desc(cseq):
    cseq = [int(i) for i in cseq.split(";")]
    out = {}
    while len(cseq):
        cseq, out = parse_out(cseq, out)
    return out

for l in LS_COLORS:
    k = l[0]
    if len(l) < 2:
        continue
    v = l[1]
    if k == "ln" and v == "target":
        continue
    if k in COMP_NAMES:
        if COMP_NAMES[k] != None:
            if "generic" not in out:
                out["generic"] = {}
            out["generic"][COMP_NAMES[k]] = make_type_desc(v)
    else:
        out[k.lstrip("*")] = make_type_desc(v)

print(json.dumps(out, indent=4))
