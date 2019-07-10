#include "ls_dir_table.h"

#include "stretchy_buffer.h"

#include "ls_printer.h"
#include "ls_state.h"

// Default sort key for dir tables. Might add more later, always a good idea to
// make it expandable.
static int ls_dir_table_sort_key_default(const void* a, const void* b) {
    struct ls_dir* A = *((struct ls_dir**)a);
    struct ls_dir* B = *((struct ls_dir**)b);
    return strcoll(
        A->name,
        B->name
    );
}

// Sort a directory table. Currently only sorts alphabetically.
void ls_dir_table_sort(struct ls_state* state, struct ls_dir_table* dirs_obj) {
    if (state) {
        if (dirs_obj->count == 0) {
            return;
        }
        qsort(
            dirs_obj->children_ptr,
            dirs_obj->count,
            sizeof(struct ls_dir*),
            ls_dir_table_sort_key_default
        );
    }
}

// Add a directory to the dir table. This is also how command line arguments are
// processed. Any extraneous arguments are deemed input directories and are
// passed to this function so that they can be sanitized and later evaluated.
void ls_dir_table_add(struct ls_state* state, struct ls_dir_table* dir_table, char* name, int cmdline, ino_t inode) {
    struct ls_dir* curdir = ls_dir_make(state, name, cmdline, inode);
    if (dir_table->count == 0) {
        // Should the table be appended to for the first time, ensure that a
        // call to sb_push will not fail due to an uninitialized pointer.
        dir_table->children_ptr = NULL;
    }
    sb_push(dir_table->children_ptr, curdir);
    dir_table->count++;
}

// Evaluate the directory table. Consider this the "main loop" of the program.
// It calls itself recursively for "-R". One of the big advantages to using a
// system of dynamic allocations for directory management like this is that
// recursion is super easy (statically this would be a nightmare, at least for
// me.) The rlvl parameter may be used for indentation at some point (like a
// "compressed recursive" directory listing mode where directory listings are
// printed inline) but now it is just used for preventing a newline at the very
// end which is a weird-ass side effect of the way I programmed in recursion.
void ls_dir_table_eval(struct ls_state* state, struct ls_dir_table* this_dir_tbl, int rlvl) {
    // Pre-sort the directories so that they are listed in the correct order
    ls_dir_table_sort(state, this_dir_tbl);
    for (unsigned int i=0;i<this_dir_tbl->count;i++) {
        struct ls_dir* cur_dir = this_dir_tbl->children_ptr[i];
        int invalid = 0;
        for (int i=0;i<rlvl;i++) {
            if (state->path[i] == cur_dir->id) {
                invalid = 1;
                break;
            }
        }
        if (invalid) {
            break;
        }
        // Populate the directory with files
        ls_dir_get_files(state, cur_dir, 0);
        // Sort it
        ls_dir_sort(state, cur_dir);
        // Run the preprocessor, if needed.
        ls_preproc_files(state, cur_dir);
        // Print the files.
        ls_print_files(state, cur_dir, 0);
        // Recursively print the next set of directories, if needed.
        if (state->options.recursive) {
            // Prepare the files for recursion. (eliminate .. and .)
            ls_dir_prep_recur(state, cur_dir);
            // Free the directory
            ls_dir_free_files(state, cur_dir);
            ls_printer_putln(state);
            ls_dir_table_eval(state, &(cur_dir->tbl), rlvl+1);
        } else {
            ls_dir_free_files(state, cur_dir);
            if (i != this_dir_tbl->count-1) {
                ls_printer_putln(state);
            }
        }
        ls_dir_free_rest(state, cur_dir); 
    }
}