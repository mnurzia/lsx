#include "ls_format.h"

// Set the name of a format.
void ls_format_name(struct ls_format* fmt, char* n) {
    fmt->name = n;
}

// Set the flags of a format.
void ls_format_flags(struct ls_format* fmt, int f) {
    fmt->flags = f;
}

// Set a flag of a format.
void ls_format_flag(struct ls_format* fmt, int f) {
    fmt->flags |= f;
}

// Set the icon of a format.
void ls_format_icon(struct ls_format* fmt, char* i) {
    fmt->icon = i;
}

// Set a solid foreground color of a format.
void ls_format_fgsol(struct ls_format* fmt, struct ls_color c) {
    fmt->fg.type = LS_COLOR_TYPE_SOLID;
    fmt->fg.colors[0] = c;
}

// Set a solid background color of a format.
void ls_format_bgsol(struct ls_format* fmt, struct ls_color c) {
    fmt->bg.type = LS_COLOR_TYPE_SOLID;
    fmt->bg.colors[0] = c;
}
