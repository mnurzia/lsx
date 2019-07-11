#ifndef LS_DIR_H
#define LS_DIR_H

#include "ls_common.h"

#if LS_BUILD_GIT
#include "git2.h"
#endif

#include "ls_dir_table.h"
#include "ls_file.h"

#define LS_DIR_TREE_MAX 16

struct ls_state;

struct ls_dir {
    size_t count;                   // The number of files currently open
    size_t count_full;              // The number of files actually contained (--tree)
    size_t max;                     // The max files in the directory
    char* name;                     // Directory name
    char* cmdline_name;             // Name specified on the command line;
    blkcnt_t blocks;                // The number of blocks this directory has
    int cmdline;                    // Was this specified on the command line?
    struct ls_file** files_ptr;     // Holds file data.
    struct ls_dir_table tbl;        // Children directories for recursively printing
    ino_t id;                       // For "-R", allows us to detect cycles
#if LS_BUILD_GIT
    git_repository* repo;           // "--git", for listing statuses
#endif
};

struct ls_dir* ls_dir_make(struct ls_state* state, char* name, int cmdline, ino_t inode);

size_t ls_dir_get_files(struct ls_state* state, struct ls_dir* this_dir, int rlvl);

void ls_dir_free_files(struct ls_state* state, struct ls_dir* dir_obj);

void ls_dir_free_rest(struct ls_state* state, struct ls_dir* this_dir);

int ls_dir_sort_key_default(void* A, void* B);

int ls_dir_sort_key_size(void* A, void* B);

int ls_dir_sort_key_mtime(void* A, void* B);

int ls_dir_sort_key_ctime(void* A, void* B);

int ls_dir_sort_key_atime(void* A, void* B);

void ls_dir_sort(struct ls_state* state, struct ls_dir* this_dir);

void ls_dir_prep_recur(struct ls_state* state, struct ls_dir* this_dir);

#endif