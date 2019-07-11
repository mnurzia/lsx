#ifndef LS_PRINTER_H
#define LS_PRINTER_H

#include "ls_common.h"

#include "ls_dir.h"

#define LS_PRINT_BUFF_LEN 1024

enum {
    LS_PRINTER_BRANCH_NULL,
    LS_PRINTER_BRANCH_NONE,
    LS_PRINTER_BRANCH_TURN,
    LS_PRINTER_BRANCH_LEAF,
    LS_PRINTER_BRANCH_DOWN
};

struct ls_state;

struct ls_printer_state {
    char buffer[LS_PRINT_BUFF_LEN]; // Print "queue"; every 1024 bytes the buffer is flushed
    size_t size;                    // How full the buffer is
    char tree_state[128];           // Tree branch string
};

void ls_do_error(struct ls_state* state, char* mess);

void ls_do_warning(struct ls_state* state, char* mess);

void ls_do_arg_error(struct ls_state* state, char* mess);

#if LS_BUILD_GIT
void ls_do_git_error(struct ls_state* state, int giterr);

void ls_do_git_warning(struct ls_state* state, int giterr);
#endif

void ls_printer_flush(struct ls_state* state);

void ls_printer_putch(struct ls_state* state, char c);

void ls_printer_putln(struct ls_state* state);

void ls_printer_puts(struct ls_state* state, char* s);

void ls_printer_puts_justify(struct ls_state* state, char* s, int jlen);

void ls_printer_putf(struct ls_state* state, int format);

void ls_printer_putr(struct ls_state* state);

void ls_print_files(struct ls_state* state, struct ls_dir* this_dir, int rlvl);

#endif