#ifndef LS_STATE_H
#define LS_STATE_H

#include "ls_common.h"

#include "ls_uid_gid.h"
#include "ls_options.h"
#include "ls_dir_table.h"
#include "ls_preproc.h"
#include "ls_printer.h"
#include "ls_theme.h"

struct ls_state {
    struct ls_uid_cache_page* ucch; // Pointer to UID cache
    struct ls_gid_cache_page* gcch; // Pointer to GID cache
    struct option* apoptions;       // Argument parser options
    struct ls_options options;      // Command-line options
    struct ls_dir_table cmdline;    // Directories to print which were specified on the commandline
    struct ls_preproc_state pp_state;//Preprocessor state
    struct ls_printer_state pr_state;//Printer state
    struct ls_theme theme;          // Theme state
    char*  progname;                // Program name
    ino_t* path;                    // Path of inodes which is compared on each recursion to avoid cycles
};

void ls_state_init(struct ls_state* state, char* progname);

#endif