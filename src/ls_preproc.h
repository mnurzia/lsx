#ifndef LS_PREPROC_H
#define LS_PREPROC_H

#include "ls_common.h"

#include "ls_dir.h"
#include "ls_file.h"
#include "ls_format.h"

enum {
    LS_LISTING_REGULAR,             // Default format (1 per line)
    LS_LISTING_LONG,                // "-l", "-g", long listing mode, print extended info
    LS_LISTING_STREAM,              // "-m", print commas between files, also print on 1 line total
    LS_LISTING_COLUMNS,             // "-C", print in columns
    LS_LISTING_ROWS                 // "-x", print in rows
};

enum {
    LS_LINK_LSTAT,                  // Use lstat() (default)
    LS_LINK_RSTAT                   // "-L", use stat() 
};

enum {
    LS_DATE_MTIME,                  // Default date printing (mtime)
    LS_DATE_CTIME,                  // Print date as status modification time (ctime)
    LS_DATE_ATIME                   // Print date as access time (atime)
};

enum {
    LS_CLASSIFY_NONE,               // Default classification (none)
    LS_CLASSIFY_DIRECTORIES,        // Classify only DIRECTORIES (print a '/' after their name)
    LS_CLASSIFY_ALL                 // Classify everything possible
};

enum {
    LS_SIZE_REGULAR,                // Regular size printing
    LS_SIZE_HUMAN_READABLE,         // Human readable size printing
    LS_SIZE_HUMAN_READABLE_FLOAT    // Human readable size printing, float-wise
};

extern const char* ls_units_regular[9];
extern const char* ls_units_si[9];
extern const char* ls_units_canon[9];

#define LS_NSTR_MAX 20              // Max length for number strings
#define LS_DSTR_MAX 30              // Max length for date strings
#define LS_MODE_LEN 12              // Length of mode strings
#if LS_BUILD_GIT
#define LS_GS_LEN 3                 // Length of git status strings
#endif

enum {
    LS_COLUMN_STRING = 1,
    LS_COLUMN_NAME,
    LS_COLUMN_INODE,
    LS_COLUMN_MODE,
    LS_COLUMN_NLINKS,
    LS_COLUMN_OWNER,
    LS_COLUMN_GROUP,
    LS_COLUMN_SIZE_NUMBER,
    LS_COLUMN_SIZE_UNITS,
    LS_COLUMN_BLOCKS,
    LS_COLUMN_TIME,
    LS_COLUMN_DEVINFO,
#if LS_BUILD_GIT
    LS_COLUMN_GITSTATUS,
#endif
    LS_COLUMN_NAME_BORDER_LEFT,
    LS_COLUMN_NAME_BORDER_RIGHT
};

enum {
    LS_JUSTIFY_NONE,
    LS_JUSTIFY_LEFT,
    LS_JUSTIFY_RIGHT,
    LS_JUSTIFY_MERGE_LEFT,
    LS_JUSTIFY_MERGE_RIGHT
};

struct ls_preproc_column {
    int justify;
    int type;
    int width;
    char* pad_left;
    char* pad_right;
};

struct ls_preproc_page {
    char  nlinks_s[LS_NSTR_MAX];    // The number of links to the file, as a string
    char  size_s[LS_NSTR_MAX];      // The size of the file, as a string
    char* size_units;               // The size unit indicator
    char  date_s[LS_DSTR_MAX];      // The date of the file's last modification, as a string
    char  mode_s[LS_MODE_LEN];      // The file's mode, as a string
    char  inode_s[LS_NSTR_MAX];     // The file's inode, as a string
    char  blocks_s[LS_NSTR_MAX];    // The number of blocks the file occupies, as a string
    char* uname;                    // The file's owner, as a string
    char* gname;                    // The file's group, as a string
    struct ls_format* format;       // Pointer to the file's color/icon
    int   name_l;                   // The length of the file's name (used for -C)
    int   ilvl;                     // Indentation level (used for --tree)
#if LS_BUILD_GIT
    char  git_status_s[LS_GS_LEN];  // Git status
#endif
};

struct ls_preproc_state {
    int initialized;                // If the preprocessor has been initialized
    size_t count;                   // The number of preprocessor pages used currently
    size_t allocsize;               // The number of preprocessor pages available
    time_t now;                     // Used for recording date+time
    unsigned int nlinks_w;          // Widths of various columns
    unsigned int uname_w;           //  |
    unsigned int gname_w;           //  |
    unsigned int size_w;            //  |
    unsigned int date_w;            //  |
    unsigned int inode_w;           //  |
    unsigned int name_w;            //  |
    unsigned int blocks_w;          //  |
#if LS_BUILD_GIT                    //  |
    unsigned int gs_w;              //  |
#endif                              //  |
    unsigned int icon_w;            //  -
    struct ls_preproc_column* cols; // Table columns
    struct ls_preproc_page* pages;  // Pointer to preproc pages
    int ncols;                      // Number of columns to print
    int* cols_w;                    // Widths of each column
};

void ls_preproc_files(struct ls_state* state, struct ls_dir* this_dir);

#endif