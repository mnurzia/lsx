#include "ls_theme_parse.h"

#include "pdjson.h"

#include "ls_state.h"

static const char* ls_theme_border_default_names[] =  {
    "triangle",
    "triangle_thin",
    "powerline",
    "powerline_thin",
    "rounded",
    "rounded_thin",
    "triangle_up",
    "triangle_up_thin",
    "triangle_down",
    "triangle_down_thin",
    "flames",
    "flames_thin",
    "pixelated",
    "pixelated_big",
    "ice",
    "hexagons",
    "hexagons_thin",
    "trapezoid",
    NULL
};

static const char* ls_theme_border_default_left[] = {
    "\ue0b2",
    "\ue0b3",
    "\ue0b2",
    "\ue0b3",
    "\ue0b6",
    "\ue0b7",
    "\ue0be",
    "\ue0bf",
    "\ue0ba",
    "\ue0bb",
    "\ue0c2",
    "\ue0c3",
    "\ue0c5",
    "\ue0c7",
    "\ue0ca",
    "\ue0cc",
    "\ue0dd",
    "\ue0d2"
};

static const char* ls_theme_border_default_right[] = {
    "\ue0b0",
    "\ue0b1",
    "\ue0b0",
    "\ue0b1",
    "\ue0b4",
    "\ue0b5",
    "\ue0bc",
    "\ue0bd",
    "\ue0b8",
    "\ue0b9",
    "\ue0c0",
    "\ue0c1",
    "\ue0c4",
    "\ue0c6",
    "\ue0c8",
    "\ue0cc",
    "\ue0dd",
    "\ue0d4"
};

static struct ls_format* ls_theme_parse_format_from_key(struct ls_state* state, char* key, size_t sz) {
    char buf[64];
    strncpy((char*)&buf, key, MIN(62, sz));
    buf[MIN(62, sz)+1] = 0;
    for (int i = 0; i < LS_FORMAT_LAST; i++) {
        if (strcmp((char*)&buf, ls_theme_verbose_names[i]) == 0) {
            return &(state->theme.formats[i]);
        }
    }
    return NULL;
}

static struct ls_format* ls_theme_parse_extension_from_key(struct ls_state* state, char* key, size_t sz) {
    char buf[64];
    strncpy((char*)&buf, key, MIN(62, sz));
    buf[63] = 0;
    struct ls_format* out;
    HASH_FIND_STR(state->theme.extensions, (char*)&buf, out);
    if (out == NULL) {
        out = (struct ls_format*)malloc(sizeof(struct ls_format));
        char* new = malloc(strlen((char*)&buf));
        if (new == NULL) {
            ls_do_error(state, "couldn't allocate extension name");
        }
        strcpy(new, (char*)&buf);
        ls_theme_zero_format(out, new);
        out->name = new;
        HASH_ADD_STR(state->theme.extensions, name, out);
    }
    return out;
}

#define ls_theme_parse_error_s(s, j, m) ls_theme_parse_error_sd(__LINE__, (s), (j), (m))

static void ls_theme_parse_error_sd(int line, struct ls_state* state, struct json_stream* stream, const char* msg) {
    char err[512];
    snprintf(
        err,
        512,
        "(%u) JSON error at line %lu (position %lu): %s",
        line,
        json_get_lineno(stream),
        json_get_position(stream),
        msg
    );
    json_close(stream);
    ls_do_warning(state, err);
    if (state->theme.extensions != NULL) {
        struct ls_format* current;
        struct ls_format* tmp;
        HASH_ITER(hh, state->theme.extensions, current, tmp) {
            HASH_DEL(state->theme.extensions, current);
            free(current);
        }
        state->theme.extensions = NULL;
    }
    for (int i = 0; i < LS_FORMAT_LAST; i++) {
        ls_theme_zero_format(&(state->theme.formats[i]),(char*)ls_theme_verbose_names[i]);
    }
}

static void ls_theme_parse_error(struct ls_state* state, struct json_stream* stream) {
    ls_theme_parse_error_s(state, stream, json_get_error(stream));
}

const char* ls_theme_parse_json_type_s[JSON_NULL+1] = {
    "JSON_NULL",
    "JSON_ERROR",
    "JSON_DONE",
    "JSON_OBJECT",
    "JSON_OBJECT_END",
    "JSON_ARRAY",
    "JSON_ARRAY_END",
    "JSON_STRING",
    "JSON_NUMBER",
    "JSON_TRUE",
    "JSON_FALSE",
    "JSON_NULL"
};

enum ls_theme_parse_state {
    LS_TS_NULL,
    LS_TS_TYPE_LIST,
    LS_TS_TYPE_LIST_VALUE,
    LS_TS_TYPE_DESC,
    LS_TS_TYPE_DESC_ICON,
    LS_TS_TYPE_DESC_COLOR,
    LS_TS_TYPE_DESC_FORMAT,
    LS_TS_TYPE_DESC_BORDER,
    LS_TS_COLOR_ARR,
    LS_TS_COLOR_RGB,
    LS_TS_COLOR_DESC,
    LS_TS_COLOR_DESC_TYPE,
    LS_TS_LAST
};

const char* ls_theme_parse_state_type_s[LS_TS_LAST] = {
    "LS_TS_NULL",
    "LS_TS_TYPE_LIST",
    "LS_TS_TYPE_LIST_VALUE",
    "LS_TS_TYPE_DESC",
    "LS_TS_TYPE_DESC_ICON",
    "LS_TS_TYPE_DESC_COLOR",
    "LS_TS_TYPE_DESC_FORMAT",
    "LS_TS_TYPE_DESC_BORDER",
    "LS_TS_COLOR_ARR",
    "LS_TS_COLOR_RGB",
    "LS_TS_COLOR_DESC",
    "LS_TS_COLOR_DESC_TYPE"
};

struct ls_theme_parse_stack_frame {
    enum ls_theme_parse_state t;
    int f;
    char* key_name;
    size_t key_len;
};

#define STACK_PUSH(a, f) stack[stack_ptr++] = (struct ls_theme_parse_stack_frame){a, f, NULL, 0}
#define STACK_POP() (stack_ptr--)
#define STACK_PEEK() (stack[stack_ptr-1])
#define STACK_PEEKN(n) (stack[stack_ptr-1-n])
#define STACK_FLAG (STACK_PEEK().f)
#define STACK_READ_STR()    { \
                                STACK_PEEK().key_name = (char*)json_get_string(&stream, &(STACK_PEEK().key_len)); \
                            }
#define TEST_STR(s) (strncmp(STACK_PEEK().key_name, (s), STACK_PEEK().key_len) == 0)
#define HOLD(a) iter_hold = a;

static int ls_theme_parse_fgetc(void *arg) {
    return fgetc((FILE*)arg);
}

void ls_theme_parse_file(struct ls_state* state, FILE* fp) {
    struct json_stream stream;
    json_open(&stream, ls_theme_parse_fgetc, fp, 0);
    enum json_type j = JSON_NULL;
    struct ls_theme_parse_stack_frame stack[32];
    int stack_ptr = 0;
    int iter_hold = 0;
    struct ls_format* current_format = NULL;
    struct ls_color_iter* current_color_iter = NULL;
    struct ls_color current_color = ls_color_none();
    int current_flag = 0;
    //struct ls_color_iter* current_color;
    //struct ls_color current_rgb;
    for (int i=0;i<32;i++) {
        stack[i].t = LS_TS_NULL;
        stack[i].f = 0;
        stack[i].key_name = NULL;
        stack[i].key_len = 0;
    }
    STACK_PUSH(LS_TS_NULL, 0);
    while (j != JSON_DONE) {
        if (iter_hold == 0) {
            j = json_next(&stream);
        } else {
            iter_hold--;
        }
        if (j == JSON_ERROR) {
            ls_theme_parse_error(state, &stream);
            return;
        }
        //printf("%u %s %i %s %i %i\n",STACK_PEEK().t, ls_theme_parse_state_type_s[STACK_PEEK().t], stack_ptr, ls_theme_parse_json_type_s[j], iter_hold, STACK_FLAG);
        switch (STACK_PEEK().t) {
            // Initial state
            case LS_TS_NULL: {
                switch (j) {
                    case JSON_OBJECT:
                        STACK_PUSH(LS_TS_TYPE_LIST, 0);
                        break;
                    case JSON_OBJECT_END:
                        break;
                    case JSON_DONE:
                        break;
                    default:
                        ls_theme_parse_error_s(state, &stream, "expected object");
                        return;
                }
                break;
            }
            // Found first object, default to Type List ("generic" or ".filetype")
            // Flags:
            //  1: If this is a nested generic 
            case LS_TS_TYPE_LIST: {
                switch (j) {
                    case JSON_STRING:
                        STACK_READ_STR();
                        current_format = NULL;
                        if (STACK_PEEK().key_len == 0) {
                            ls_theme_parse_error_s(state, &stream, "expected string of length >1");
                            return;
                        }
                        if (STACK_PEEK().key_name[0] == '#') {
                            current_format = ls_theme_parse_format_from_key(
                                state,
                                (char*)(&STACK_PEEK().key_name[1]),
                                STACK_PEEK().key_len-1
                            );
                        } else {
                            current_format = ls_theme_parse_extension_from_key(
                                state,
                                (char*)STACK_PEEK().key_name,
                                STACK_PEEK().key_len
                            );
                        }
                        STACK_PUSH(LS_TS_TYPE_LIST_VALUE, 0);
                        break;
                    case JSON_OBJECT_END:
                        STACK_POP();
                        HOLD(1);
                        break;
                    default:
                        ls_theme_parse_error_s(state, &stream, "expected string");
                        return;
                }
                break;
            }
            // Found first object + name
            case LS_TS_TYPE_LIST_VALUE: {
                switch (j) {
                    case JSON_OBJECT:
                        STACK_PUSH(LS_TS_TYPE_DESC, 0);
                        break;
                    case JSON_OBJECT_END:
                        STACK_POP();
                        break;
                    case JSON_STRING:
                        STACK_READ_STR();
                        current_color = ls_color_parse(STACK_PEEK().key_name, STACK_PEEK().key_len);
                        current_color_iter = &(current_format->fg);
                        if (current_color.flag != LS_COLOR_FLAG_INVALID) {
                            sb_free(current_color_iter->colors);
                            current_color_iter->colors = NULL;
                            current_color_iter->type = LS_COLOR_TYPE_SOLID;
                            sb_push(current_color_iter->colors, current_color);
                        } else {
                            ls_theme_parse_error_s(state, &stream, "invalid color");
                            return;
                        }
                        STACK_POP();
                        break;
                    default:
                        ls_theme_parse_error_s(state, &stream, "expected object or string");
                        break;
                }
                break;
            }
            // Found type description
            case LS_TS_TYPE_DESC: {
                switch (j) {
                    case JSON_STRING: {
                        STACK_READ_STR();
                        if (TEST_STR("icon")) {
                            STACK_PUSH(LS_TS_TYPE_DESC_ICON, 0);
                        } else if (TEST_STR("fg") || TEST_STR("color")) {
                            current_color_iter = &(current_format->fg);
                            STACK_PUSH(LS_TS_TYPE_DESC_COLOR, 0);
                        } else if (TEST_STR("bg")) {
                            current_color_iter = &(current_format->bg);
                            STACK_PUSH(LS_TS_TYPE_DESC_COLOR, 0);
                        } else if (TEST_STR("icon_fg") || TEST_STR("ifg")) {
                            current_color_iter = &(current_format->ifg);
                            current_format->flags |= LS_FORMAT_FLAG_HAS_ICON_COLOR;
                            STACK_PUSH(LS_TS_TYPE_DESC_COLOR, 0);
                        } else if (TEST_STR("icon_bg") || TEST_STR("ibg")) {
                            current_color_iter = &(current_format->ibg);
                            current_format->flags |= LS_FORMAT_FLAG_HAS_ICON_COLOR;
                            STACK_PUSH(LS_TS_TYPE_DESC_COLOR, 0);
                        } else if (TEST_STR("ext_fg") || TEST_STR("extension_fg") || TEST_STR("efg")) {
                            current_color_iter = &(current_format->efg);
                            current_format->flags |= LS_FORMAT_FLAG_HAS_EXT_COLOR;
                            STACK_PUSH(LS_TS_TYPE_DESC_COLOR, 0);
                        } else if (TEST_STR("ext_bg") || TEST_STR("extension_bg") || TEST_STR("ebg")) {
                            current_color_iter = &(current_format->ebg);
                            current_format->flags |= LS_FORMAT_FLAG_HAS_EXT_COLOR;
                            STACK_PUSH(LS_TS_TYPE_DESC_COLOR, 0);
                        } else if (TEST_STR("bold")) {
                            current_flag = LS_FORMAT_FLAG_BOLD;
                            STACK_PUSH(LS_TS_TYPE_DESC_FORMAT, 0);
                        } else if (TEST_STR("underline") || TEST_STR("under")) {
                            current_flag = LS_FORMAT_FLAG_UNDERLINE;
                            STACK_PUSH(LS_TS_TYPE_DESC_FORMAT, 0);
                        } else if (TEST_STR("italic")) {
                            current_flag = LS_FORMAT_FLAG_ITALIC;
                            STACK_PUSH(LS_TS_TYPE_DESC_FORMAT, 0);
                        } else if (TEST_STR("strikethrough") || TEST_STR("strike")) {
                            current_flag = LS_FORMAT_FLAG_STRIKETHROUGH;
                            STACK_PUSH(LS_TS_TYPE_DESC_FORMAT, 0);
                        } else if (TEST_STR("reverse") || TEST_STR("reverse_video")) {
                            current_flag = LS_FORMAT_FLAG_REVERSE;
                            STACK_PUSH(LS_TS_TYPE_DESC_FORMAT, 0);
                        } else if (TEST_STR("faint")) {
                            current_flag = LS_FORMAT_FLAG_FAINT;
                            STACK_PUSH(LS_TS_TYPE_DESC_FORMAT, 0);
                        } else if (TEST_STR("blink") || TEST_STR("flash")) {
                            current_flag = LS_FORMAT_FLAG_BLINK;
                            STACK_PUSH(LS_TS_TYPE_DESC_FORMAT, 0);
                        } else if (TEST_STR("double_underline") || TEST_STR("double_under")) {
                            current_flag = LS_FORMAT_FLAG_DOUBLE_UNDERLINE;
                            STACK_PUSH(LS_TS_TYPE_DESC_FORMAT, 0);
                        } else if (TEST_STR("fast_blink")) {
                            current_flag = LS_FORMAT_FLAG_FAST_BLINK;
                            STACK_PUSH(LS_TS_TYPE_DESC_FORMAT, 0);
                        } else if (TEST_STR("border")) {
                            STACK_PUSH(LS_TS_TYPE_DESC_BORDER, 0);
                        } else {
                            ls_theme_parse_error_s(state, &stream, 
                                "expected one of: "
                                "    'icon', 'fg' (aka 'color'), 'bg',\n"
                                "    'icon_fg' (aka 'ifg'), 'icon_bg' (aka 'ibg'),\n"
                                "    'ext_fg' (aka 'efg' or 'extension_fg'),\n"
                                "    'ext_bg' (aka 'ebg' or 'extension_bg'),\n"
                                "    'bold', 'underline' (aka 'under'), 'italic',\n"
                                "    'strikethrough' (aka 'strike'),\n"
                                "    'reverse' (aka 'reverse_video'), 'faint',\n"
                                "    'blink' (aka 'flash'), 'fast_blink', or\n"
                                "    'double_underline' (aka 'double_under')");
                            return;
                        }
                        break;
                    }
                    case JSON_OBJECT_END:
                        STACK_POP();
                        HOLD(1);
                        break;
                    default:
                        ls_theme_parse_error_s(state, &stream, "expected string");
                        return;
                }
                break;
            }
            // Found icon string
            case LS_TS_TYPE_DESC_ICON: {
                switch (j) {
                    case JSON_STRING: {
                        size_t len;
                        const char* src = json_get_string(&stream, &len);
                        char* dst = (char*)malloc(len+1);
                        strncpy(dst, src, len);
                        dst[len] = 0;
                        current_format->icon = dst;
                        STACK_POP();
                        break;
                    }
                    default:
                        ls_theme_parse_error_s(state, &stream, "expected string");
                        return;
                }
                break;
            }
            // Found color descriptor
            // Flags:
            //  0: Has the child been set to be parsed yet?
            //  1: Should sb for the colors be freed at the end?
            case LS_TS_TYPE_DESC_COLOR: {
                if (!(STACK_FLAG & 1)) {
                    switch (j) {
                        case JSON_NUMBER:
                            current_color = ls_color_lofi((uint8_t)json_get_number(&stream));
                            sb_free(current_color_iter->colors);
                            current_color_iter->colors = NULL;
                            current_color_iter->type = LS_COLOR_TYPE_SOLID;
                            sb_push(current_color_iter->colors, current_color);
                            STACK_POP();
                            break;
                        case JSON_STRING:
                            STACK_READ_STR();
                            current_color = ls_color_parse(STACK_PEEK().key_name, STACK_PEEK().key_len);
                            if (current_color.flag != LS_COLOR_FLAG_INVALID) {
                                sb_free(current_color_iter->colors);
                                current_color_iter->colors = NULL;
                                current_color_iter->type = LS_COLOR_TYPE_SOLID;
                                sb_push(current_color_iter->colors, current_color);
                            } else {
                                ls_theme_parse_error_s(state, &stream, "invalid color");
                                return;
                            }
                            STACK_POP();
                            break;
                        case JSON_NULL:
                            current_color = ls_color_none();
                            sb_free(current_color_iter->colors);
                            current_color_iter->colors = NULL;
                            current_color_iter->type = LS_COLOR_TYPE_SOLID;
                            sb_push(current_color_iter->colors, current_color);
                            STACK_POP();
                            break;
                        case JSON_ARRAY:
                            STACK_FLAG |= 1;
                            current_color = ls_color_rgb(0, 0, 0);
                            if (json_peek(&stream) == JSON_ARRAY) {
                                STACK_FLAG |= 2;
                                STACK_PUSH(LS_TS_COLOR_ARR, 0);
                            } else {
                                STACK_PUSH(LS_TS_COLOR_RGB, 0);
                            }
                            break;
                        case JSON_OBJECT:
                            STACK_FLAG |= 1;
                            STACK_PUSH(LS_TS_COLOR_DESC, 0);
                            break;
                        default:
                            ls_theme_parse_error_s(state, &stream, "expected number, string, array, or object");
                            return;
                    }
                } else {
                    switch (j) {
                        case JSON_ARRAY_END: // return from LS_TS_COLOR_RGB
                            if (!(STACK_FLAG & 2)) {
                                sb_free(current_color_iter->colors);
                                current_color_iter->colors = NULL;
                                current_color_iter->type = LS_COLOR_TYPE_SOLID;
                                sb_push(current_color_iter->colors, current_color);
                            }
                            STACK_POP();
                            break;
                        case JSON_OBJECT_END:
                            STACK_POP();
                            break;
                        default:
                            ls_theme_parse_error_s(state, &stream, "expected array or object end");
                            return;
                    }
                }
                break;
            }
            // Found array of colors
            //  0: Has the child been set to be parsed yet?
            //  1: Is this not the first color?
            case LS_TS_COLOR_ARR: {
                if (!(STACK_FLAG & 1)) {
                    switch (j) {
                        case JSON_ARRAY:
                            STACK_FLAG |= 1;
                            current_color = ls_color_rgb(0, 0, 0);
                            STACK_PUSH(LS_TS_COLOR_RGB, 0);
                            break;
                        case JSON_ARRAY_END:
                            STACK_POP();
                            HOLD(1);
                            break;
                        default:
                            ls_theme_parse_error_s(state, &stream, "expected array");
                            return;
                    }
                } else {
                    switch (j) {
                        case JSON_ARRAY_END:
                            if (!(STACK_FLAG & 2)) {
                                sb_free(current_color_iter->colors);
                                current_color_iter->colors = NULL;
                                STACK_FLAG |= 2;
                            }
                            sb_push(current_color_iter->colors, current_color);
                            STACK_FLAG &= ~(1);
                            break;
                        default:
                            ls_theme_parse_error_s(state, &stream, "expected array end");
                            return;
                    }
                }
                break;
            }
            // Found first object, default to Type List ("generic" or ".filetype")
            // Flags:
            //  Counter (states 0-2, r, g, b components, respectively; state 3, finish)
            case LS_TS_COLOR_RGB: {
                switch (j) {
                    case JSON_NUMBER:
                        if (STACK_FLAG >= 3) {
                            ls_theme_parse_error_s(state, &stream, "expected only 1 or only 3 elements in color array, but got 4+");
                            return;
                        }
                        if (STACK_FLAG == 0) {
                            current_color.r = (uint8_t)json_get_number(&stream);
                        } else if (STACK_FLAG == 1) {
                            current_color.g = (uint8_t)json_get_number(&stream);
                        } else if (STACK_FLAG == 2) {
                            current_color.b = (uint8_t)json_get_number(&stream);
                        }
                        if (STACK_FLAG < 3) {
                            STACK_FLAG += 1;
                        }
                        break;
                    case JSON_ARRAY_END:
                        if (STACK_FLAG == 1) {
                            // if this is a LOFI array...
                            current_color.flag = LS_COLOR_FLAG_LOFI;
                        } else if (STACK_FLAG < 3) {
                            ls_theme_parse_error_s(state, &stream, "expected only 1 or only 3 elements in color array, but got neither");
                            return;
                        }
                        HOLD(1);
                        STACK_POP();
                        break;
                    default:
                        ls_theme_parse_error_s(state, &stream, "expected number");
                        return;
                }
                break;
            }
            // Found an extended color object
            case LS_TS_COLOR_DESC: {
                switch (j) {
                    case JSON_STRING:
                        STACK_READ_STR();
                        if (TEST_STR("type")) {
                            STACK_PUSH(LS_TS_COLOR_DESC_TYPE, 0);
                        } else if (TEST_STR("color")) {
                            STACK_PUSH(LS_TS_TYPE_DESC_COLOR, 0);
                        } else {
                            ls_theme_parse_error_s(state, &stream, "expected one of: 'type' or 'color'");
                        }
                        break;
                    case JSON_OBJECT_END:
                        STACK_POP();
                        HOLD(1);
                        break;
                    default:
                        ls_theme_parse_error_s(state, &stream, "expected string");
                        return;
                }
                break;
            }
            // Found the type key in the extended color object
            case LS_TS_COLOR_DESC_TYPE: {
                switch (j) {
                    case JSON_STRING:
                        STACK_READ_STR();
                        if (TEST_STR("solid")) {
                            current_color_iter->type = LS_COLOR_TYPE_SOLID;
                        } else if (TEST_STR("stripe")) {
                            current_color_iter->type = LS_COLOR_TYPE_STRIPE;
                        } else if (TEST_STR("random")) {
                            current_color_iter->type = LS_COLOR_TYPE_RANDOM;
                        } else if (TEST_STR("gradient")) {
                            current_color_iter->type = LS_COLOR_TYPE_GRADIENT;
                        } else {
                            ls_theme_parse_error_s(state, &stream, "expected one of: 'solid', 'stripe', 'random', or 'gradient'");
                        }
                        STACK_POP();
                        break;
                    default:
                        ls_theme_parse_error_s(state, &stream, "expected string");
                        return;
                }
                break;
            }
            // Found a format flag name
            case LS_TS_TYPE_DESC_FORMAT: {
                switch (j) {
                    case JSON_TRUE:
                        current_format->flags |= current_flag;
                        STACK_POP();
                        break;
                    case JSON_FALSE:
                        current_format->flags &= ~(current_flag);
                        STACK_POP();
                        break;
                    default:
                        ls_theme_parse_error_s(state, &stream, "expected true or false");
                        return;
                }
                break;
            }
            // Found a border name
            case LS_TS_TYPE_DESC_BORDER: {
                switch (j) {
                    case JSON_STRING:
                        STACK_READ_STR();
                        int idx = 0;
                        while (ls_theme_border_default_names[idx]) {
                            if (
                                strncmp(
                                    STACK_PEEK().key_name,
                                    ls_theme_border_default_names[idx],
                                    STACK_PEEK().key_len
                                ) == 0
                            ) {
                                current_format->flags |= 
                                    LS_FORMAT_FLAG_HAS_BORDER |
                                    LS_FORMAT_FLAG_HAS_BORDER_LEFT |
                                    LS_FORMAT_FLAG_HAS_BORDER_RIGHT;
                                current_format->border_left = (char*)ls_theme_border_default_left[idx];
                                current_format->border_right = (char*)ls_theme_border_default_right[idx];
                                break;
                            }
                            idx++;
                        }
                        if (!(ls_theme_border_default_names[idx])) {
                            ls_theme_parse_error_s(state, &stream, "unknown border default preset");
                            return;
                        }
                        STACK_POP();
                        break;
                    default:
                        ls_theme_parse_error_s(state, &stream, "expected string");
                        return;
                }
                break;
            }
            // Last catch
            case LS_TS_LAST: {
                break;
            }
        }
    }
    json_close(&stream);
}

#undef STACK_PUSH
#undef STACK_POP
#undef STACK_PEEK
#undef STACK_PEEKN
#undef STACK_FLAG
#undef STACK_READ_STR
#undef TEST_STR
#undef HOLD