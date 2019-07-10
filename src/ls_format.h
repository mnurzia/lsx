#ifndef LS_FORMAT_H
#define LS_FORMAT_H

#include "stretchy_buffer.h"
#include "uthash.h"

#include "ls_color.h"

#define LS_FORMAT_FLAG_BOLD             0x0001
#define LS_FORMAT_FLAG_UNDERLINE        0x0002
#define LS_FORMAT_FLAG_ITALIC           0x0004
#define LS_FORMAT_FLAG_STRIKETHROUGH    0x0008
#define LS_FORMAT_FLAG_REVERSE          0x0010
#define LS_FORMAT_FLAG_FAINT            0x0020
#define LS_FORMAT_FLAG_BLINK            0x0040 // don't even
#define LS_FORMAT_FLAG_DOUBLE_UNDERLINE 0x0080
#define LS_FORMAT_FLAG_RESET            0x0100
#define LS_FORMAT_FLAG_VALID            0x0200
#define LS_FORMAT_FLAG_HAS_EXT_COLOR    0x0400
#define LS_FORMAT_FLAG_HAS_ICON_COLOR   0x0800
#define LS_FORMAT_FLAG_FAST_BLINK       0x1000
#define LS_FORMAT_FLAG_HAS_BORDER       0x2000
#define LS_FORMAT_FLAG_HAS_BORDER_LEFT  0x4000
#define LS_FORMAT_FLAG_HAS_BORDER_RIGHT 0x8000

struct ls_format {
    char* name;                 // Empty for formats, used for extensions
    int flags;                  // Format flags (underline, bold, italic, etc.)
    char* icon;                 // Icon string (should *resolve* to one char)
    char* border_left;          // Left border character
    char* border_right;         // Right border character
    struct ls_color_iter fg;    // Foreground iterator
    struct ls_color_iter bg;    // Background iterator
    struct ls_color_iter ifg;   // Icon foreground
    struct ls_color_iter ibg;   // Icon background
    struct ls_color_iter efg;   // Extension foreground
    struct ls_color_iter ebg;   // Extension background
    UT_hash_handle hh;          // Hash table util
};

// Set the name of a format.
void ls_format_name(struct ls_format* fmt, char* n);

// Set the flags of a format.
void ls_format_flags(struct ls_format* fmt, int f);

// Set a flag of a format.
void ls_format_flag(struct ls_format* fmt, int f);

// Set the icon of a format.
void ls_format_icon(struct ls_format* fmt, char* i);

// Set a solid foreground color of a format.
void ls_format_fgsol(struct ls_format* fmt, struct ls_color c);

// Set a solid background color of a format.
void ls_format_bgsol(struct ls_format* fmt, struct ls_color c);

#endif
