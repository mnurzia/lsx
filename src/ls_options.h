#ifndef LS_OPTIONS_H
#define LS_OPTIONS_H

#include "ls_common.h"
#include "ls_preproc.h"

struct ls_state;

struct ls_program_option {
    struct option apoption;         // The option as it is passed to getopt_long
    const char* description;        // The description of this option (not passed to getopt_long)
};

struct ls_options {
    int all_files;                  // "-a", list .. and . and files starting with .
    int almost_all_files;           // "-A", list files starting with ".", but not .. and .
    int classify_type;              // Should the files be classified, if so how
    int print_inodes;               // "-i", print the serial number (inode number) of each file
    int listing_type;               // What type of listing - long, regular, stream, etc.
    int preprocess_data;            // Should the lengths of each column of long mode be computed?
    int print_blocks;               // Should the block size be printed on the "total: %i" line?
    int print_headers;              // Should the directory's name be printed before the listing?
    int print_owner;                // Should the owner's name be printed in the listing?
    int print_group;                // Should the group's name be printed in the listing?
    int link_type;                  // For "-L", stat() takes precedence over lstat()
    int (*sort_key)(void*, void*);  // Sort key function, for qsort
    int stat_files;                 // Do the files actually need to be statted?
    int date_type;                  // What value to use for the date - mtime, ctime, etc.
    int print_ugid_numbers;         // "-n", print numeric uid and gid instead of names
    int sort_reverse;               // "-r", reverse the sort order afterwards
    int print_force_oneline;        // "-1", force entries to be on one line each
    unsigned int ncols;             // Number of columns to use for "-C", etc.
    int recursive;                  // "-R", print DIRECTORIES recursively
    int format_type;                // "--color", whether to print with color always, never, or automatically
    int print_icons;                // "--icons", whether to print an icon before filenames
    int resolve_links;              // "-l", whether to resolve links so they can be printed later
    int print_block_numbers;        // "-s", print block numbers before inode, etc.
    int size_type;                  // "-h", how to format sizes
    int size_power;                 // "-h", "--size-si", "--size-float", power to use for human-readable sizes
    const char** size_units;        // whether to use "k" or "K"
    int color_mode;                 // 1 for 8-256 color mode, 0 for 24-bit
    char* config_file;              // ls_colors (later, config, maybe???) json file
    int listing_tree;               // "--tree", whether to print tree listing
    int listing_tree_max_depth;     // "--tree-max-depth", how many levels to print for "--tree" (0-16)
    size_t listing_tree_max_files;  // "--tree-max-files", how many entries to print for "--tree" (0+)
#if LS_BUILD_GIT
    int git_open;                   // "--git", whether or not to open a repository
    int git_init;                   // "--git", whether or not git has been initialized
#endif
};


void ls_options_get(struct ls_state* state, int argc, char** argv);

void ls_options_prepare(struct ls_state* state);

#endif