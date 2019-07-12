#include "ls_color.h"

#include "stretchy_buffer.h"

// Holds a named color; used when parsing a color STRING from the theme file
struct ls_color_named {
    char* name;                     // The color name (e.g. gray93)
    uint8_t color_idx;              // the 256-color index
};

struct ls_color_named ls_named_colors[256];

uint8_t ls_named_colors_len[36];

void ls_color_iter_add(struct ls_color_iter* in, struct ls_color a) {
    sb_push(in->colors,a);
}

struct ls_color ls_color_iter_stripe(struct ls_color_iter* it, int idx) {
    return it->colors[idx%sb_count(it->colors)];
}

struct ls_color ls_color_iter_random(struct ls_color_iter* it) {
    return it->colors[rand()%sb_count(it->colors)];
}

struct ls_color ls_color_lerp(struct ls_color a, struct ls_color b, float t) {
    if (a.flag == LS_COLOR_FLAG_LOFI || b.flag == LS_COLOR_FLAG_LOFI) {
        return ls_color_none();
    }
    struct ls_color ret;
    ret.flag = LS_COLOR_FLAG_RGB;
    ret.r = LERP(a.r,b.r,t);
    ret.g = LERP(a.g,b.g,t);
    ret.b = LERP(a.b,b.b,t);
    return ret;
}

struct ls_color ls_color_iter_gradient(struct ls_color_iter* it, int idx, int high) {
    float color_ratio = ((float)idx)/((float)high);
    int count = sb_count(it->colors);
    float color_ratio_idx = color_ratio*(count-1);
    // Nothing much here, I just really like how these two statements line up.
    int low_idx = (int)floor(color_ratio_idx);
    int high_idx = (int)ceil(color_ratio_idx);
    float lerp_t = color_ratio_idx-(float)low_idx;
    return ls_color_lerp(it->colors[low_idx], it->colors[high_idx], lerp_t);
}

struct ls_color ls_color_next(struct ls_color_iter* it, int idx, int high) {
    switch (it->type) {
        case LS_COLOR_TYPE_SOLID:
            return it->colors[0];
        case LS_COLOR_TYPE_STRIPE:
            return ls_color_iter_stripe(it, idx);
        case LS_COLOR_TYPE_RANDOM:
            return ls_color_iter_random(it);
        case LS_COLOR_TYPE_GRADIENT:
            return ls_color_iter_gradient(it, idx, high);
        default:
            return ls_color_none();
    }
}

struct ls_color ls_color_rgb(uint8_t r, uint8_t g, uint8_t b) {
    struct ls_color o;
    o.flag = LS_COLOR_FLAG_RGB;
    o.r = r;
    o.g = g;
    o.b = b;
    return o;
}

struct ls_color ls_color_lofi(uint8_t a) {
    struct ls_color o;
    o.flag = LS_COLOR_FLAG_LOFI;
    o.r = a;
    o.g = 0;
    o.b = 0;
    return o;
}

struct ls_color ls_color_none() {
    struct ls_color o;
    o.flag = LS_COLOR_FLAG_NONE;
    o.r = 0;
    o.g = 0;
    o.b = 0;
    return o;   
}

struct ls_color ls_color_invalid() {
    struct ls_color o;
    o.flag = LS_COLOR_FLAG_INVALID;
    o.r = 0;
    o.g = 0;
    o.b = 0;
    return o;   
}

static uint8_t ls_color_parse_hex(char in) {
    switch (in) {
        case '0' :
        case '1' :
        case '2' :
        case '3' :
        case '4' :
        case '5' :
        case '6' :
        case '7' :
        case '8' :
        case '9' : return in - '0';
        case 'a' :
        case 'b' :
        case 'c' :
        case 'd' :
        case 'e' :
        case 'f' : return in - 'a' + 10;
        case 'A' :
        case 'B' :
        case 'C' :
        case 'D' :
        case 'E' :
        case 'F' : return in - 'A' + 10;
        default  : return 16;
    }
}

struct ls_color ls_color_parse(char* in, size_t l) {
    if (l == 0) {
        return ls_color_invalid();
    }
    if (in[0] == '#') {
        if (l == 7) {
            uint8_t r0, r1, g0, g1, b0, b1, r, g, b;
            r0 = ls_color_parse_hex(in[1]);
            r1 = ls_color_parse_hex(in[2]);
            g0 = ls_color_parse_hex(in[3]);
            g1 = ls_color_parse_hex(in[4]);
            b0 = ls_color_parse_hex(in[5]);
            b1 = ls_color_parse_hex(in[6]);
            if (r0 == 16 || r1 == 16 || g0 == 16 || g1 == 16 || b0 == 16 || b1 == 16) {
                return ls_color_invalid();
            }
            r = (r0 << 4) + r1;
            g = (g0 << 4) + g1;
            b = (b0 << 4) + b1;
            return ls_color_rgb(r, g, b);
        } else if (l == 4) {
            uint8_t r0, g0, b0, r, g, b;
            r0 = ls_color_parse_hex(in[1]);
            g0 = ls_color_parse_hex(in[2]);
            b0 = ls_color_parse_hex(in[3]);
            if (r0 == 16 || g0 == 16 || b0 == 16) {
                return ls_color_invalid();
            }
            r = (r0 << 4) + r0;
            g = (g0 << 4) + g0;
            b = (b0 << 4) + b0;
            return ls_color_rgb(r, g, b);
        } else if (l == 2) {
            uint8_t g0, g;
            g0 = ls_color_parse_hex(in[1]);
            if (g0 == 16) {
                return ls_color_invalid();
            }
            g = (g0 << 4) + g0;
            return ls_color_rgb(g, g, g);
        } else {
            return ls_color_invalid();
        }
    } else {
        if (l <= 18) {
            uint8_t idx = (l - 1) << 1;
            uint8_t start = ls_named_colors_len[idx];
            uint8_t amt = ls_named_colors_len[idx+1];
            struct ls_color_named* fin = NULL;
            if (amt == 0) {
                return ls_color_invalid();
            }
            for (int i = 0; i < amt; i++) {
                if (strncmp(in, ls_named_colors[start+i].name, l) == 0) {
                    fin = &(ls_named_colors[start+i]);
                    break;
                }
            }
            if (fin) {
                return ls_color_lofi(fin->color_idx);
            } else {
                return ls_color_invalid();
            }
        } else {
            return ls_color_invalid();
        }
    }
}

// This would probably be better as a binary search but whatever. i learned
// about binary search after writing this anyways.
struct ls_color_named ls_named_colors[256] = {
    {"red"               ,   9}, {"tan"               , 180}, {"navy"              ,   4}, {"teal"              ,   6},
    {"grey"              ,   8}, {"lime"              ,  10}, {"blue"              ,  12}, {"aqua"              ,  14},
    {"red3"              , 124}, {"red3"              , 160}, {"red1"              , 196}, {"black"             ,   0},
    {"green"             ,   2}, {"olive"             ,   3}, {"white"             ,  15}, {"grey0"             ,  16},
    {"blue3"             ,  19}, {"blue3"             ,  20}, {"blue1"             ,  21}, {"cyan3"             ,  43},
    {"cyan2"             ,  50}, {"cyan1"             ,  51}, {"plum4"             ,  96}, {"gold3"             , 142},
    {"pink3"             , 175}, {"plum3"             , 176}, {"gold3"             , 178}, {"plum2"             , 183},
    {"pink1"             , 218}, {"plum1"             , 219}, {"gold1"             , 220}, {"grey3"             , 232},
    {"grey7"             , 233}, {"maroon"            ,   1}, {"purple"            ,   5}, {"silver"            ,   7},
    {"yellow"            ,  11}, {"green4"            ,  28}, {"green3"            ,  34}, {"green3"            ,  40},
    {"green1"            ,  46}, {"grey37"            ,  59}, {"purple"            ,  93}, {"wheat4"            , 101},
    {"grey53"            , 102}, {"purple"            , 129}, {"grey63"            , 139}, {"grey69"            , 145},
    {"orchid"            , 170}, {"violet"            , 177}, {"khaki3"            , 185}, {"grey84"            , 188},
    {"khaki1"            , 228}, {"wheat1"            , 229}, {"grey11"            , 234}, {"grey15"            , 235},
    {"grey19"            , 236}, {"grey23"            , 237}, {"grey27"            , 238}, {"grey30"            , 239},
    {"grey35"            , 240}, {"grey39"            , 241}, {"grey42"            , 242}, {"grey46"            , 243},
    {"grey50"            , 244}, {"grey54"            , 245}, {"grey58"            , 246}, {"grey62"            , 247},
    {"grey66"            , 248}, {"grey70"            , 249}, {"grey74"            , 250}, {"grey78"            , 251},
    {"grey82"            , 252}, {"grey85"            , 253}, {"grey89"            , 254}, {"grey93"            , 255},
    {"fuchsia"           ,  13}, {"darkred"           ,  52}, {"purple4"           ,  54}, {"purple4"           ,  55},
    {"purple3"           ,  56}, {"orange4"           ,  58}, {"darkred"           ,  88}, {"orange4"           ,  94},
    {"yellow4"           , 100}, {"yellow4"           , 106}, {"yellow3"           , 148}, {"orange3"           , 172},
    {"yellow3"           , 184}, {"yellow2"           , 190}, {"hotpink"           , 205}, {"hotpink"           , 206},
    {"salmon1"           , 209}, {"orchid2"           , 212}, {"orchid1"           , 213}, {"orange1"           , 214},
    {"yellow1"           , 226}, {"grey100"           , 231}, {"navyblue"          ,  17}, {"darkblue"          ,  18},
    {"darkcyan"          ,  36}, {"skyblue3"          ,  74}, {"skyblue2"          , 111}, {"skyblue1"          , 117},
    {"magenta3"          , 127}, {"hotpink3"          , 132}, {"magenta3"          , 163}, {"magenta3"          , 164},
    {"magenta2"          , 165}, {"hotpink3"          , 168}, {"hotpink2"          , 169}, {"thistle3"          , 182},
    {"magenta2"          , 200}, {"magenta1"          , 201}, {"thistle1"          , 225}, {"darkgreen"         ,  22},
    {"deeppink4"         ,  53}, {"steelblue"         ,  67}, {"cadetblue"         ,  72}, {"cadetblue"         ,  73},
    {"seagreen3"         ,  78}, {"seagreen2"         ,  83}, {"seagreen1"         ,  84}, {"seagreen1"         ,  85},
    {"deeppink4"         ,  89}, {"deeppink4"         , 125}, {"indianred"         , 131}, {"rosybrown"         , 138},
    {"darkkhaki"         , 143}, {"deeppink3"         , 161}, {"deeppink3"         , 162}, {"indianred"         , 167},
    {"honeydew2"         , 194}, {"deeppink2"         , 197}, {"deeppink1"         , 198}, {"deeppink1"         , 199},
    {"cornsilk1"         , 230}, {"turquoise4"        ,  30}, {"turquoise2"        ,  45}, {"blueviolet"        ,  57},
    {"slateblue3"        ,  61}, {"slateblue3"        ,  62}, {"royalblue1"        ,  63}, {"steelblue3"        ,  68},
    {"steelblue1"        ,  75}, {"palegreen3"        ,  77}, {"steelblue1"        ,  81}, {"darkviolet"        ,  92},
    {"lightpink4"        ,  95}, {"slateblue1"        ,  99}, {"palegreen3"        , 114}, {"lightgreen"        , 119},
    {"lightgreen"        , 120}, {"palegreen1"        , 121}, {"darkviolet"        , 128}, {"lightcyan3"        , 152},
    {"palegreen1"        , 156}, {"lightpink3"        , 174}, {"mistyrose3"        , 181}, {"lightcyan1"        , 195},
    {"orangered1"        , 202}, {"indianred1"        , 203}, {"indianred1"        , 204}, {"darkorange"        , 208},
    {"lightcoral"        , 210}, {"sandybrown"        , 215}, {"lightpink1"        , 217}, {"mistyrose1"        , 224},
    {"dodgerblue3"       ,  26}, {"dodgerblue2"       ,  27}, {"dodgerblue1"       ,  33}, {"chartreuse4"       ,  64},
    {"chartreuse3"       ,  70}, {"chartreuse3"       ,  76}, {"aquamarine3"       ,  79}, {"chartreuse2"       ,  82},
    {"aquamarine1"       ,  86}, {"darkmagenta"       ,  90}, {"darkmagenta"       ,  91}, {"chartreuse2"       , 112},
    {"chartreuse1"       , 118}, {"aquamarine1"       , 122}, {"darkorange3"       , 130}, {"greenyellow"       , 154},
    {"darkorange3"       , 166}, {"deepskyblue4"      ,  23}, {"deepskyblue4"      ,  24}, {"deepskyblue4"      ,  25},
    {"springgreen4"      ,  29}, {"deepskyblue3"      ,  31}, {"deepskyblue3"      ,  32}, {"springgreen3"      ,  35},
    {"deepskyblue2"      ,  38}, {"deepskyblue1"      ,  39}, {"springgreen3"      ,  41}, {"springgreen2"      ,  42},
    {"springgreen2"      ,  47}, {"springgreen1"      ,  48}, {"mediumpurple"      , 104}, {"darkseagreen"      , 108},
    {"mediumorchid"      , 134}, {"lightsalmon3"      , 137}, {"navajowhite3"      , 144}, {"lightsalmon3"      , 173},
    {"lightyellow3"      , 187}, {"lightsalmon1"      , 216}, {"navajowhite1"      , 223}, {"lightseagreen"     ,  37},
    {"darkturquoise"     ,  44}, {"mediumpurple4"     ,  60}, {"darkseagreen4"     ,  65}, {"darkseagreen4"     ,  71},
    {"mediumpurple3"     ,  97}, {"mediumpurple3"     ,  98}, {"lightskyblue3"     , 109}, {"lightskyblue3"     , 110},
    {"darkseagreen3"     , 115}, {"mediumorchid3"     , 133}, {"mediumpurple2"     , 135}, {"darkgoldenrod"     , 136},
    {"mediumpurple2"     , 140}, {"mediumpurple1"     , 141}, {"darkseagreen3"     , 150}, {"darkseagreen2"     , 151},
    {"lightskyblue1"     , 153}, {"darkseagreen2"     , 157}, {"darkseagreen1"     , 158}, {"mediumorchid1"     , 171},
    {"darkseagreen1"     , 193}, {"mediumorchid1"     , 207}, {"paleturquoise4"    ,  66}, {"cornflowerblue"    ,  69},
    {"darkslategray2"    ,  87}, {"lightslategrey"    , 103}, {"lightslateblue"    , 105}, {"darkslategray3"    , 116},
    {"darkslategray1"    , 123}, {"lightsteelblue"    , 147}, {"paleturquoise1"    , 159}, {"palevioletred1"    , 211},
    {"mediumturquoise"   ,  80}, {"darkolivegreen3"   , 107}, {"darkolivegreen3"   , 113}, {"mediumvioletred"   , 126},
    {"lightsteelblue3"   , 146}, {"darkolivegreen3"   , 149}, {"darkolivegreen2"   , 155}, {"lightgoldenrod3"   , 179},
    {"lightgoldenrod2"   , 186}, {"lightsteelblue1"   , 189}, {"darkolivegreen1"   , 191}, {"darkolivegreen1"   , 192},
    {"lightgoldenrod2"   , 221}, {"lightgoldenrod2"   , 222}, {"lightgoldenrod1"   , 227}, {"mediumspringgreen" ,  49}
};

uint8_t ls_named_colors_len[36] = {
      0,  0,   0,  0,   0,  0,   0,  2,   2,  9,  11, 22,
     33, 43,  76, 22,  98, 17, 115, 22, 137, 31, 168, 17,
    185, 22, 207, 23, 230, 10, 240, 15,   0,  0, 255,  1
};