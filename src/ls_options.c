#include "ls_options.h"

#include "ls_state.h"
// NOTE: ls_state.h is only included here because otherwise there would be a
// weird circular dependency.

#define LS_OPTION_HELP (CHAR_MIN - 2)
#define LS_OPTION_COLOR (CHAR_MIN - 3)
#define LS_OPTION_ICONS (CHAR_MIN - 4)
#define LS_OPTION_SIZE_SI (CHAR_MIN - 5)
#define LS_OPTION_SIZE_FLOATS (CHAR_MIN - 6)
#define LS_OPTION_SIZE_CNC (CHAR_MIN - 7)
#define LS_OPTION_COLOR_MODE (CHAR_MIN - 8)
#define LS_OPTION_CONFIG_FILE (CHAR_MIN - 9)
#define LS_OPTION_TREE (CHAR_MIN - 10)
#define LS_OPTION_TREE_MAX_DEPTH (CHAR_MIN - 11)
#define LS_OPTION_TREE_MAX_FILES (CHAR_MIN - 12)
#if LS_BUILD_GIT
    #define LS_OPTION_GIT (CHAR_MIN - 13)
#endif
//#define LS_OPTION_FORMAT (CHAR_MIN - 14)

struct ls_program_option ls_program_options[] = {
    {{"all", no_argument, NULL, 'a'},
        "list entries starting with . (this includes . and ..)"},
    {{"sort-ctime", no_argument, NULL, 'c'},
        "sort by file status modification time (ctime), most recent first"},
    {{"generic", no_argument, NULL, 'f'},
        "do not sort, enable -aU"},
    {{"size-humans", no_argument, NULL, 'h'},
        "print sizes in human-readable format (B, K, M, G, T...)"},
    {{"omit-group", no_argument, NULL, 'g'},
        "don't print the group's name/ID in long listing mode"},
    {{"inode", no_argument, NULL, 'i'},
        "print the serial number (inode number) of each file"},
    {{"list-long", no_argument, NULL, 'l'},
        "use a long listing format"},
    {{"list-stream", no_argument, NULL, 'm'},
        "stream output: separate files by ',', separate dirs by '\\n'"},
    {{"print-ids", no_argument, NULL, 'n'},
        "print numeric UIDs/GIDs in long listing mode"},    
    {{"omit-owner", no_argument, NULL, 'o'},
        "don't print the owner's name/ID in long listing mode"},
    {{"classify-dirs", no_argument, NULL, 'p'},
        "append a '/' to each directory"},
    {{"sort-reverse", no_argument, NULL, 'r'},
        "reverse the printing order of the files after sorting"},
    {{"print-size", no_argument, NULL, 's'},
        "print the number of blocks used before the filename"},
    {{"sort-time", no_argument, NULL, 't'},
        "sort by file modification time, most recent first"},
    {{"sort-atime", no_argument, NULL, 'u'},
        "sort by file access time (atime), most recent first"},
    {{"width", required_argument, NULL, 'w'},
        "width of output device (used for -Cx)"},
    {{"list-rows", no_argument, NULL, 'x'},
        "output formatted as rows"},
    {{"almost-all", no_argument, NULL, 'A'},
        "do not list implied . and .. but list files starting with ."},
    {{"list-columns", no_argument, NULL, 'C'},
        "output formatted as columns, with each column sized dynamically " \
        "according to the content of each one"},
    {{"classify", no_argument, NULL, 'F'},
        "append character indicator (one of */@|=>#%) to object entries:" \
        " *: executable," \
        " /: directory," \
        " @: link," \
        " |: fifo," \
        " =: socket," \
        " >: door," \
        " #: block device," \
        " %: char device"},
    {{"dereference-command-line", no_argument, NULL, 'H'},
        "follow symbolic links listed on the command line"},
    {{"dereference", no_argument, NULL, 'L'},
        "when showing file information for a symbolic link, " \
        "show information for the file the link references " \
        "rather than for the link itself"},
    {{"recursive", no_argument, NULL, 'R'},
        "print files recursively"},
    {{"sort-size", no_argument, NULL, 'S'},
        "sort by file size, largest first"},
    {{"sort-none", no_argument, NULL, 'U'},
        "do not sort; list entries in directory order (as they appear from readdir())"},
    {{"version", no_argument, NULL, 'V'},
        "print version text and exit"},
    {{"list-one", no_argument, NULL, '1'},
        "force printing to be one entry per line"},
    {{"color", required_argument, NULL, LS_OPTION_COLOR},
        "when to print colors, valid options are 'auto', 'on', or 'off'"},
    {{"color-mode", required_argument, NULL, LS_OPTION_COLOR_MODE},
        "color depth, valid options are 'lofi' or 'rgb'"},
    {{"icons", required_argument, NULL, LS_OPTION_ICONS},
        "when to print icons, valid options are 'auto', 'on', or 'off'"},
    {{"size-si", no_argument, NULL, LS_OPTION_SIZE_SI},
        "when in -h, use powers of 1000, not 1024"},
    {{"size-canonical", no_argument, NULL, LS_OPTION_SIZE_CNC},
        "use KB, MB, etc."},
    {{"size-floats", no_argument, NULL, LS_OPTION_SIZE_FLOATS},
        "like -h, but print as floats"},
    {{"config-file", required_argument, NULL, LS_OPTION_CONFIG_FILE},
        ".json file in replacement of LS_COLORS"},
    {{"tree", optional_argument, NULL, LS_OPTION_TREE},
        "print files in a tree listing, valid options are 'auto', 'on', or 'off'"},
    {{"tree-max-depth", required_argument, NULL, LS_OPTION_TREE_MAX_DEPTH},
        "maximum depth to inline display files for --tree (no greater than 16)"},
    {{"tree-max-files", required_argument, NULL, LS_OPTION_TREE_MAX_FILES},
        "maximum amount of files to inline display for --tree (0 for all)"},
#if LS_BUILD_GIT
    {{"git", no_argument, NULL, LS_OPTION_GIT},
        "display git info after/next to files in -l"},
#endif
/**
    {{"format", required_argument, NULL, LS_OPTION_FORMAT},
        "format specifiers, see below"},
**/
    {{"help", no_argument, NULL, LS_OPTION_HELP},
        "print this help text and exit"},
    {{NULL,0,NULL,0},NULL},
};

const int ls_program_options_count = (sizeof(ls_program_options)/sizeof(struct ls_program_option))-1;

const char* ls_program_options_string = 
    "acfghilmnoprstuw:xACFHLRSUV1";

const char* ls_program_usage = 
    "Usage: ls [OPTIONS]... [FILES]...";

const char* ls_program_description = 
    "List information about the FILEs (the current directory by default).";

const char* ls_program_help = 
"lsx, by default, attempts to be compliant with the POSIX standard and with\n"
" GNU ls; thus output will look dull at first. For a more modern look, try:\n"
"  alias ls='lsx --color=auto --icons=auto --color-mode=rgb --size-floats\n"
"  --size-canonical -w 120 -CFh'\n"
"Other useful aliases:\n"
"  alias la='ls -a'\n"
"  alias ll='ls -l'\n"
"  alias lt='ls --tree'\n";

const char* ls_program_version = "lsx version %i.%i.%i";

const char* ls_program_version_with_git = " (-DBUILD_GIT)";

void ls_options_prepare(struct ls_state* state) {
    // Allocate space for getopt option list
    state->apoptions = malloc(sizeof(struct option)*(ls_program_options_count+1));
    if (state->apoptions == NULL) {
        ls_do_error(state, "Couldn't prepare program options");
    }
    for (int i=0;i<(ls_program_options_count+1);i++) {
        // Copy program options to getopt options
        state->apoptions[i].name = ls_program_options[i].apoption.name;
        state->apoptions[i].has_arg = ls_program_options[i].apoption.has_arg;
        state->apoptions[i].flag = ls_program_options[i].apoption.flag;
        state->apoptions[i].val = ls_program_options[i].apoption.val;
        //memcpy(&(state->apoptions[i]), &popts[i].apoption, sizeof(struct option));
    }
}

static void ls_options_print_usage() {
    printf("%s\n",ls_program_usage);
}

static void ls_options_print_version() {
    printf(ls_program_version,LS_VERSION_MAJOR,LS_VERSION_MINOR,LS_VERSION_PATCH);
#if LS_BUILD_GIT
    printf(ls_program_version_with_git);
#endif
    printf("\n");
}

static void ls_print_help_text() {
    ls_options_print_usage();
    printf("\n");
    printf("%s\n",ls_program_description);
    printf("\n");
    printf("Options:\n");
    for (int i=0;i<ls_program_options_count;i++) {
        struct ls_program_option* curopt = &ls_program_options[i];
        if (curopt->apoption.name == NULL) {
            printf("  -%c                    ", curopt->apoption.val);
        } else if (curopt->apoption.val < CHAR_MIN) {
            printf("      --%s",curopt->apoption.name);
            int len = strlen(curopt->apoption.name);
            if (len > 16) {
                printf("\n                        ");
            } else {
                for (int i = 0; i < 16 - len; i++) {
                    printf(" ");
                }
            }
        } else {
            printf("  -%c, --%s",curopt->apoption.val,curopt->apoption.name);
            int len = strlen(curopt->apoption.name);
            if (len > 16) {
                printf("\n                        ");
            } else {
                for (int i = 0; i < 16 - len; i++) {
                    printf(" ");
                }
            }
        }
        char* start = (char*)curopt->description;
        char* word = start;
        char* end = start;
        int col = 0;
        do {
            if (isspace(*end)) {
                word = end;
            }
            if (col != 55) {
                col++;
                end++;
            } else {
                while (start != word) {
                    printf("%c", *start);
                    start++;
                }
                start = word;
                word = start;
                col = 2;
                printf("\n                          ");
            }
        } while (*end);
        while (start != end) {
            printf("%c", *start);
            start++;
        }
        printf("\n");
    }
    printf("\n");
    printf("%s",ls_program_help);
}

enum ls_options_format_parse_state {
    LS_OF_GND,
    LS_OF_STR,
    LS_OF_FSPEC
};

// Saved for a later version

/* #define LS_COLUMN_NULL (struct ls_preproc_column){0, LS_JUSTIFY_LEFT, 0, NULL, NULL}

void ls_options_format_parse(struct ls_state* state, char* fmt) {
    char* p = fmt;
    enum ls_options_format_parse_state s;
    struct ls_preproc_column current;
    struct ls_preproc_column* out = NULL;
    do {
        switch (s) {
            case LS_OF_GND:
                switch (*p) {
                    case '%':
                        s = LS_OF_FSPEC;
                        current = LS_COLUMN_NULL;
                        break;
                    case '\0':
                        break;
                    default:
                        s = LS_OF_STR;
                        current = LS_COLUMN_NULL;
                        current.type = LS_COLUMN_STRING;
                        sb_push(current.pad_left, *p);
                        break;
                }
                break;
            case LS_OF_STR:
                switch (*p) {
                    case '%':
                        s = LS_OF_FSPEC;
                        sb_push(current.pad_left, '\0');
                        sb_push(out, current);
                        break;
                    case '\0':
                        sb_push(current.pad_left, '\0');
                        sb_push(out, current);
                        break;
                    default:
                        sb_push(current.pad_left, *p);
                        break;
                }
                break;
            case LS_OF_FSPEC:
                switch (*p) {
                    case '<':   // Justify left
                        current.justify = LS_JUSTIFY_LEFT;
                        break;
                    case '>':   // Justify right
                        current.justify = LS_JUSTIFY_RIGHT;
                        break;
                    case 'n':   // filename
                        s = LS_OF_GND;
                        if (current.justify == LS_JUSTIFY_LEFT) {
                            current.type = LS_COLUMN_NAME_BORDER_LEFT;
                            current.justify = LS_JUSTIFY_RIGHT;
                            sb_push(out, current);
                            current = LS_COLUMN_NULL;
                            current.type = LS_COLUMN_NAME;
                            current.justify = LS_JUSTIFY_LEFT;
                            sb_push(out, current);
                            current = LS_COLUMN_NULL;
                            current.type = LS_COLUMN_NAME_BORDER_RIGHT;
                            current.justify = LS_JUSTIFY_MERGE_LEFT;
                            sb_push(out, current);
                        } else {
                            current.type = LS_COLUMN_NAME_BORDER_LEFT;
                            current.justify = LS_JUSTIFY_MERGE_RIGHT;
                            sb_push(out, current);
                            current = LS_COLUMN_NULL;
                            current.type = LS_COLUMN_NAME;
                            current.justify = LS_JUSTIFY_RIGHT;
                            sb_push(out, current);
                            current = LS_COLUMN_NULL;
                            current.type = LS_COLUMN_NAME_BORDER_RIGHT;
                            current.justify = LS_JUSTIFY_LEFT;
                            sb_push(out, current);
                        }
                        break;
                    case 'i':   // inode
                        s = LS_OF_GND;
                        current.type = LS_COLUMN_INODE;
                        sb_push(out, current);
                        break;
                    case 'm':   // mode
                        s = LS_OF_GND;
                        current.type = LS_COLUMN_MODE;
                        sb_push(out, current);
                        break;
                    case 'l':   // nlinks
                        s = LS_OF_GND;
                        current.type = LS_COLUMN_NLINKS;
                        sb_push(out, current);
                        break;
                    case 'o':   // owner
                        s = LS_OF_GND;
                        current.type = LS_COLUMN_OWNER;
                        sb_push(out, current);
                        break;
                    case 'g':   // group
                        s = LS_OF_GND;
                        current.type = LS_COLUMN_GROUP;
                        sb_push(out, current);
                        break;
                    case 's':   // size
                        s = LS_OF_GND;
                        current.type = LS_COLUMN_SIZE_NUMBER;
                        if (current.justify == LS_JUSTIFY_LEFT) {
                            current.justify = LS_JUSTIFY_RIGHT;
                            sb_push(out, current);
                            current = LS_COLUMN_NULL;
                            current.type = LS_COLUMN_SIZE_UNITS;
                            current.justify = LS_JUSTIFY_LEFT;
                        } else {
                            current.justify = LS_JUSTIFY_MERGE_RIGHT;
                            sb_push(out, current);
                            current = LS_COLUMN_NULL;
                            current.type = LS_COLUMN_SIZE_UNITS;
                            current.justify = LS_JUSTIFY_RIGHT;
                        }
                        sb_push(out, current);
                        break;
                    case '\0':
                        ls_do_arg_error(state, "can't end format string in-between format specifier");
                        goto cleanup;
                    default:
                        ls_do_arg_error(state, "invalid format specifier character");
                        break;
                }
        }
        p++;
    } while (*p);
cleanup:
    if (current.pad_left) {
        sb_free(current.pad_left);
    }
    if (current.pad_right) {
        sb_free(current.pad_right);
    }
    if (out) {
        for (int i = 0; i < sb_count(out)) {
            if (out[i].pad_left) {
                sb_free(out.pad_left);
            }
            if (out[i].pad_right) {
                sb_free(out.pad_right);
            }
        }
        sb_free(out);
    }
} */

void ls_options_get(struct ls_state* state, int argc, char** argv) {
    int toption;
    while (
        (
            toption = getopt_long(
                argc, 
                argv, 
                ls_program_options_string, 
                state->apoptions, 
                NULL
            )
        ) != -1) {
        switch (toption) {
            case 'a': 
                state->options.all_files = 1;
                break;
            case 'c':
                state->options.sort_key = ls_dir_sort_key_ctime;
                state->options.date_type = LS_DATE_CTIME;
                state->options.stat_files = 1;
                break;
            case 'f':
                state->options.sort_key = NULL;
                state->options.all_files = 1;
                break;
            case 'g':
                state->options.print_group = 0;
                break;
            case 'h':
                state->options.size_type = LS_SIZE_HUMAN_READABLE;
                break;
            case 'i':
                state->options.print_inodes = 1;
                state->options.stat_files = 1;
                break;
            case 'l':
                state->options.stat_files = 1;
                state->options.listing_type = LS_LISTING_LONG;
                state->options.preprocess_data = 1;
                state->options.print_blocks = 1;
                state->options.resolve_links = 1;
                break;
            case 'm':
                state->options.listing_type = LS_LISTING_STREAM;
                break;
            case 'n':
                state->options.print_ugid_numbers = 1;
                break;
            case 'o':
                state->options.print_owner = 0;
                break;
            case 'p':
                if (isatty(STDOUT_FILENO)) {
                    state->options.classify_type = LS_CLASSIFY_DIRECTORIES;
                }
                break;
            case 'r':
                state->options.sort_reverse = 1;
                break;
            case 's':
                state->options.stat_files = 1;
                state->options.print_block_numbers = 1;
                break;
            case 't':
                state->options.sort_key = ls_dir_sort_key_mtime;
                state->options.date_type = LS_DATE_MTIME;
                state->options.stat_files = 1;
                break;
            case 'u':
                state->options.sort_key = ls_dir_sort_key_atime;
                state->options.date_type = LS_DATE_ATIME;
                state->options.stat_files = 1;
                break;
            case 'w': {
                errno = 0;
                char* endptr;
                int ncols = (int)strtol(optarg, &endptr, 10);
                if (*endptr == '\0') {
                    if (ncols < 0 || ncols == 0) {
                        ncols = 1;
                    } else {
                        state->options.ncols = ncols;
                    }
                } else {
                    ls_do_arg_error(state,"--width: Invalid number");
                }
                break;
            }
            case 'x':
                state->options.listing_type = LS_LISTING_ROWS;
                break;
            case 'A':
                state->options.almost_all_files = 1;
                break;
            case 'C':
                state->options.listing_type = LS_LISTING_COLUMNS;
                break;
            case 'F':
                if (isatty(STDOUT_FILENO)) {
                    state->options.stat_files = 1;
                    state->options.classify_type = LS_CLASSIFY_ALL;
                }
                break;
            case 'H':
                state->options.link_type = LS_LINK_LSTAT;
                break;
            case 'L':
                state->options.link_type = LS_LINK_RSTAT;
                break;
            case 'R':
                state->options.recursive = 1;
                state->options.stat_files = 1;
                state->options.print_headers = 1;
                state->options.preprocess_data = 1;
                state->options.resolve_links = 1;
                break;
            case 'S':
                state->options.sort_key = ls_dir_sort_key_size;
                state->options.stat_files = 1;
                break;
            case 'U':
                state->options.sort_key = NULL;
                break;
            case 'V':
                ls_options_print_version();
                exit(0);
                break;
            case '1':
                state->options.print_force_oneline = 1;
                state->options.listing_type = LS_LISTING_REGULAR;
                break;
            case LS_OPTION_HELP:
                ls_print_help_text();
                exit(0);
                break;
            case LS_OPTION_COLOR:
                if (strcmp(optarg,"off") == 0) {
                    state->options.format_type = 0;
                    break;
                } else if (strcmp(optarg,"on") == 0) {
                    state->options.format_type = 1;
                    state->options.stat_files = 1;
                    break;
                } else if (strcmp(optarg,"auto") == 0) {
                    if (isatty(STDOUT_FILENO)) {
                        state->options.format_type = 1;
                        state->options.stat_files = 1;
                    }
                    break;
                } else {
                    ls_do_arg_error(state,"--color: Invalid option; options are: off,on,auto");
                    break;
                }
                state->options.print_icons = 0;
                break;
            case LS_OPTION_ICONS:
                if (strcmp(optarg,"off") == 0) {
                    state->options.print_icons = 0;
                    break;
                } else if (strcmp(optarg,"on") == 0) {
                    state->options.print_icons = 1;
                    state->options.stat_files = 1;
                    break;
                } else if (strcmp(optarg,"auto") == 0) {
                    if (isatty(STDOUT_FILENO)) {
                        state->options.print_icons = 1;
                        state->options.stat_files = 1;
                    }
                    break;
                } else {
                    ls_do_arg_error(state,"--icons: Invalid option; options are: off,on,auto");
                    break;
                }
                state->options.print_icons = 0;
                break;
            case LS_OPTION_SIZE_SI:
                state->options.size_power = 1000;
                state->options.size_units = ls_units_si;
                state->options.size_type = LS_SIZE_HUMAN_READABLE;
                break;
            case LS_OPTION_SIZE_FLOATS:
                state->options.size_type = LS_SIZE_HUMAN_READABLE_FLOAT;
                break;
            case LS_OPTION_SIZE_CNC:
                state->options.size_units = ls_units_canon;
                break;
            case LS_OPTION_COLOR_MODE:
                if (strcmp(optarg,"lofi") == 0) {
                    state->options.color_mode = 0;
                    break;
                } else if (strcmp(optarg,"rgb") == 0) {
                    state->options.color_mode = 1;
                    break;
                } else {
                    ls_do_arg_error(state,"--color-mode: Invalid option; options are: lofi,rgb");
                    break;
                }
                break;
            case LS_OPTION_CONFIG_FILE:
                state->options.config_file = optarg;
                break;
            case LS_OPTION_TREE:
                if (optarg == NULL) {
                    if (isatty(STDOUT_FILENO)) {
                        state->options.listing_tree = 1;
                        state->options.stat_files = 1;
                    }
                } else {
                    if (strcmp(optarg,"off") == 0) {
                        state->options.listing_tree = 0;
                        break;
                    } else if (strcmp(optarg,"on") == 0) {
                        state->options.listing_tree = 1;
                        state->options.stat_files = 1;
                        break;
                    } else if (strcmp(optarg,"auto") == 0) {
                        if (isatty(STDOUT_FILENO)) {
                            state->options.listing_tree = 1;
                            state->options.stat_files = 1;
                        }
                        break;
                    } else {
                        ls_do_arg_error(state,"--tree: Invalid option; options are: off,on,auto");
                        break;
                    }
                }
                state->options.print_icons = 0;
                break;
            case LS_OPTION_TREE_MAX_DEPTH: {
                errno = 0;
                char* endptr;
                int maxdepth = (int)strtol(optarg, &endptr, 10);
                if (*endptr == '\0') {
                    if (maxdepth < 0 || maxdepth > 16) {
                        ls_do_arg_error(state,"--tree-max-depth: Number out of range");
                    } else {
                        state->options.listing_tree_max_depth = maxdepth;
                    }
                } else {
                    ls_do_arg_error(state,"--tree-max-depth: Invalid number");
                }
                break;
            }
            case LS_OPTION_TREE_MAX_FILES: {
                errno = 0;
                char* endptr;
                int maxfiles = (int)strtol(optarg, &endptr, 10);
                if (*endptr == '\0') {
                    state->options.listing_tree_max_files = maxfiles;
                } else {
                    ls_do_arg_error(state,"--tree-max-files: Invalid number");
                }
                break;
            }
#if LS_BUILD_GIT
            case LS_OPTION_GIT: {
                if (!state->options.git_init) {
                    git_libgit2_init();
                    state->options.git_open = 1;
                    state->options.git_init = 1;
                    state->options.stat_files = 1;
                }
                break;
            }
#endif
/**
            case LS_OPTION_FORMAT: {
                if (optarg == NULL) {
                    ls_do_arg_error(state,"--format: Invalid string");
                } else {
                    printf("%s", optarg);
                }
                break;
            }
**/
            case '?':
            default:
                ls_options_print_usage();
                exit(1);
                break;
        }
    }
    if (optind < argc) {
        do {
            ls_dir_table_add(state,&(state->cmdline),argv[optind],1,0);
        }
        while ( ++optind < argc);
    }
    if (state->cmdline.count > 1) {
        if (state->options.listing_type != LS_LISTING_STREAM) {
            state->options.print_headers = 1;
        }
    }
    if (state->cmdline.count == 0) {
        ls_dir_table_add(state,&(state->cmdline),".",1,0);
    }
    if (state->options.format_type || state->options.print_icons) {
        ls_theme_load(state);
    }
}