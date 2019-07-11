#ifndef LS_COLOR_H
#define LS_COLOR_H

#include "ls_common.h"

// Enum containing all of the possible values for the "type" field of
// struct ls_color_iter
enum {
    LS_COLOR_TYPE_SOLID,            // Solid color, same color throughout.
    LS_COLOR_TYPE_STRIPE,           // Stripe color, follows pattern of colors list.
    LS_COLOR_TYPE_RANDOM,           // Random color, follows no pattern but chooses from color list.
    LS_COLOR_TYPE_GRADIENT          // Gradient, a gradient between the colors.
};

// Enum defining the possible color types
enum {
    LS_COLOR_FLAG_NONE,             // No color (0m)?
    LS_COLOR_FLAG_INVALID,          // Invalid color
    LS_COLOR_FLAG_LOFI,             // 256 color?
    LS_COLOR_FLAG_RGB               // Or 888 RGB?
};

// Holds a color; I didn't make this a union because I liked that it was able
// to be exactly 4 bytes and would probably get passed smoothly in registers
// without optimization. This is an example of premature optimization, though.
// Was this really the best design choice?
struct ls_color {
    uint8_t r;                      // R intensity (24-bit color mode)
    uint8_t g;                      // G intensity (24-bit color mode)
    uint8_t b;                      // B intensity (24-bit color mode)
    uint8_t flag;                   // Is this a lo-fi or hi-fi color
};

// Basically holds all of the information necessary to pull colors for a string.
// The colors field is typically an stb_sb and contains all of the colors that
// were given in the theme config.
struct ls_color_iter {
    int type;                       // Type of color mode
    struct ls_color* colors;        // The colors which it contains (>=1)
};

// Add a color to an iterator
void ls_color_iter_add(struct ls_color_iter* in, struct ls_color a);

// Get a striped color from an iterator (type==LS_COLOR_TYPE_STRIPE)
struct ls_color ls_color_iter_stripe(struct ls_color_iter* it, int idx);

// Get a random color from an iterator (type==LS_COLOR_TYPE_RANDOM)
struct ls_color ls_color_iter_random(struct ls_color_iter* it);

// Get a gradient color from an iterator (type==LS_COLOR_TYPE_GRADIENT)
struct ls_color ls_color_iter_gradient(struct ls_color_iter* it, int idx, int high);

// Get the next color from an iterator
struct ls_color ls_color_next(struct ls_color_iter* it, int idx, int high);

// Get a color
struct ls_color ls_color_rgb(uint8_t r, uint8_t g, uint8_t b);

// Get a lofi color
struct ls_color ls_color_lofi(uint8_t a);

// Get the null color
struct ls_color ls_color_none();

// Get the invalid color
struct ls_color ls_color_invalid();

// Parse out a color
struct ls_color ls_color_parse(char* in, size_t l);

#endif