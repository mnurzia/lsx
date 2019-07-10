#ifndef LS_DIR_TABLE_H
#define LS_DIR_TABLE_H

#include "ls_common.h"

struct ls_state;
struct ls_dir;

struct ls_dir_table {
    //struct ls_dir* children;      // Children directories
    size_t count;                   // Number of children
    struct ls_dir** children_ptr;   // Pointers for sorting dirs
};

void ls_dir_table_sort(struct ls_state* state, struct ls_dir_table* dirs_obj);

void ls_dir_table_add(struct ls_state* state, struct ls_dir_table* dir_table, char* name, int cmdline, ino_t inode);

void ls_dir_table_eval(struct ls_state* state, struct ls_dir_table* this_dir_tbl, int rlvl);

#endif