#ifndef LS_FILE_H
#define LS_FILE_H

#include "ls_common.h"

// This should have been named "ls_entry", oh well

struct ls_file {
    char* name;                     // File name
    char* ext;                      // Extension pointer (points to part of name)
    char* link_to;                  // Where does the file link to?
    int valid;                      // If the file has valid data or not
    char  clschar[2];               // Append class char ("-F")
    struct ls_preproc_page* pp_page;// Preprocessor page for this file (if applicible)
    struct stat status;             // The stat structure that the above are derived from
    struct ls_dir* child;           // Child directory
#if LS_BUILD_GIT
    unsigned int git_status;        // Git status flags ("--git")
#endif
};

#endif