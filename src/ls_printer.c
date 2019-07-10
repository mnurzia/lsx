#include "ls_printer.h"

#if LS_BUILD_GIT

#include "git2.h"

#endif

#include "ls_state.h"

void ls_printer_flush(struct ls_state*);

void ls_do_error(struct ls_state* state, char* mess) {
    ls_printer_flush(state);
    fprintf(stderr,"%s: ",state->progname);
    perror(mess);
    exit(1);
}

#if LS_BUILD_GIT

void ls_do_git_error(struct ls_state* state, int giterr) {
    const git_error *e = giterr_last();
    fprintf(stderr,"%s: libgit2: %d/%d: %s\n", state->progname, giterr, e->klass, e->message);
    exit(1);
}

void ls_do_git_warning(struct ls_state* state, int giterr) {
    const git_error *e = giterr_last();
    fprintf(stderr,"%s: libgit2: %d/%d: %s\n", state->progname, giterr, e->klass, e->message);
}

#endif

void ls_do_warning(struct ls_state* state, char* mess) {
    ls_printer_flush(state);
    fprintf(stderr,"%s: %s\n",state->progname,mess);
    //perror(mess);
}

void ls_do_arg_error(struct ls_state* state, char* mess) {
    ls_printer_flush(state);
    fprintf(stderr,"%s: ",state->progname);
    fprintf(stderr,"%s\n",mess);
    exit(2);
}

void ls_printer_flush(struct ls_state* state) {
    fwrite(&(state->pr_state.buffer), state->pr_state.size, 1, stdout);
    state->pr_state.size = 0;
}

void ls_printer_putch(struct ls_state* state, char c) {
    state->pr_state.buffer[state->pr_state.size] = c;
    state->pr_state.size++;
    if (state->pr_state.size == LS_PRINT_BUFF_LEN) {
        ls_printer_flush(state);
    }
}

void ls_printer_putln(struct ls_state* state) {
    ls_printer_putch(state, '\n');
}

void ls_printer_puts(struct ls_state* state, char* s) {
    char* sp = s;
    while (*sp) {
        ls_printer_putch(state, *sp);
        sp += sizeof(char);
    }
}

void ls_printer_puts_justify_l(struct ls_state* state, int nch, int jlen) {
    for (int i=0;i<jlen-nch;i++) {
        ls_printer_putch(state,' ');
    }
}

void ls_printer_puts_justify(struct ls_state* state, char* s, int jlen) {
    int nch = strlen(s);
    ls_printer_puts_justify_l(state, nch, jlen);
}

static void ls_printer_putcolor_lofi(struct ls_state* state, uint8_t lf, int fg) {
    char buf[32];
    if (lf < 16) {
        int base;
        if (lf < 8) {
            if (fg) {
                base = 30;
            } else {
                base = 40;
            }
        } else {
            if (fg) {
                base = 90-8;
            } else {
                base = 100-8;
            }
        }
        snprintf(buf, 32, "\x1b[%um", base+lf);
    } else { // 88/256
        if (fg) {
            snprintf(buf, 32, "\x1b[38;5;%um", lf);
        } else {
            snprintf(buf, 32, "\x1b[48;5;%um", lf);
        }
    }
    ls_printer_puts(state, buf);
}

void ls_printer_putcolor(struct ls_state* state, struct ls_color color, int fg) {
    if (!state->options.color_mode) {
        if (color.flag == LS_COLOR_FLAG_LOFI) {
            ls_printer_putcolor_lofi(state, color.r, fg);
        } else {
            return;
        }
    } else {    // rgb time
        if (color.flag == LS_COLOR_FLAG_NONE) {
            return;
        } else if (color.flag == LS_COLOR_FLAG_LOFI) {
            ls_printer_putcolor_lofi(state, color.r, fg);
        } else if (color.flag == LS_COLOR_FLAG_RGB) {
            char buf[32];
            if (fg) {
                snprintf(buf, 32, "\x1b[38;2;%u;%u;%um", color.r, color.g, color.b);
            } else {
                snprintf(buf, 32, "\x1b[48;2;%u;%u;%um", color.r, color.g, color.b);
            }
            ls_printer_puts(state, buf);
        }
    }
}

#define semi() if (semi == 1) { ls_printer_putch(state, ';'); } semi = 1;

void ls_printer_putf_c(struct ls_state* state, struct ls_format* fmt, int color) {
    if ((fmt->flags & ~(LS_FORMAT_FLAG_VALID))!= 0) {
        char semi = 0;
        ls_printer_puts(state, "\x1b[");
        if (fmt->flags & LS_FORMAT_FLAG_BOLD) {
            ls_printer_puts(state, "1");
            semi();
        }
        if (fmt->flags & LS_FORMAT_FLAG_UNDERLINE) {
            semi();
            ls_printer_puts(state, "4");
        }
        if (fmt->flags & LS_FORMAT_FLAG_ITALIC) {
            semi();
            ls_printer_puts(state, "3");
        }
        if (fmt->flags & LS_FORMAT_FLAG_STRIKETHROUGH) {
            semi();
            ls_printer_puts(state, "9");
        }
        if (fmt->flags & LS_FORMAT_FLAG_REVERSE) {
            if (!(fmt->flags & LS_FORMAT_FLAG_HAS_BORDER)) {
                semi();
                ls_printer_puts(state, "7");
            }
        }
        if (fmt->flags & LS_FORMAT_FLAG_HAS_BORDER) {
            if (!(fmt->flags & LS_FORMAT_FLAG_REVERSE)) {
                semi();
                ls_printer_puts(state, "7");
            }
        }
        if (fmt->flags & LS_FORMAT_FLAG_FAINT) {
            semi();
            ls_printer_puts(state, "2");
        }
        if (fmt->flags & LS_FORMAT_FLAG_BLINK) {
            semi();
            ls_printer_puts(state, "5");
        }
        if (fmt->flags & LS_FORMAT_FLAG_DOUBLE_UNDERLINE) {
            semi();
            ls_printer_puts(state, "21");
        }
        if (fmt->flags & LS_FORMAT_FLAG_FAST_BLINK) {
            semi();
            ls_printer_puts(state, "6");
        }
        if (fmt->flags & LS_FORMAT_FLAG_RESET) {
            semi();
            ls_printer_puts(state, "0");
        }
        ls_printer_putch(state, 'm');
    }
    if (color) {
        ls_printer_putcolor(state, ls_color_next(&(fmt->fg), 0, 0), 1);
        ls_printer_putcolor(state, ls_color_next(&(fmt->bg), 0, 0), 0);
    }
}

#undef semi

void ls_printer_putf(struct ls_state* state, int format) {
    struct ls_format* fmt = &(state->theme.formats[format]);
    ls_printer_putf_c(state, fmt, 1);
}

void ls_printer_putcolor_s(
    struct ls_state* state, struct ls_color_iter* fgi, struct ls_color_iter* bgi,
    char* s, size_t len) {
    struct ls_color fgl = ls_color_none();
    struct ls_color bgl = ls_color_none();
    for (size_t i=0;i<len;i++) {
        struct ls_color fg = ls_color_next(fgi, i, len);
        struct ls_color bg = ls_color_next(bgi, i, len);
        // Optimization: check if the color is the same as what was just printed.
        // If so, no need to print a new color!
        // TODO: make this look better...
        if (i != 0) {
            if (memcmp(&fg, &fgl, sizeof(struct ls_color)) != 0) {
                ls_printer_putcolor(state, fg, 1);
            }
            if (memcmp(&bg, &bgl, sizeof(struct ls_color)) != 0) {
                ls_printer_putcolor(state, bg, 0);
            }
        } else {
            ls_printer_putcolor(state, fg, 1);
            ls_printer_putcolor(state, bg, 0);
        }
        ls_printer_putch(state, s[i]);
        fgl = fg;
        bgl = bg;
    }
}

void ls_printer_putfs_c(struct ls_state* state, struct ls_format* fmt, char* s) {
    ls_printer_putf_c(state, fmt, 0);
    size_t l = strlen(s);
    ls_printer_putcolor_s(state, &(fmt->fg), &(fmt->bg), s, l);
}

void ls_printer_putfs(struct ls_state* state, int format, char* s) {
    struct ls_format* fmt = &(state->theme.formats[format]);
    ls_printer_putfs_c(state, fmt, s);
}

void ls_printer_putr(struct ls_state* state) {
    ls_printer_puts(state, "\x1b[0m");
}

static void ls_print_dirname_header(struct ls_state* state, struct ls_dir* this_dir) {
    if (state->options.format_type == 1) {
        ls_printer_putfs(state, LS_FORMAT_HEADER_PATH, this_dir->name);
        ls_printer_putr(state);
        ls_printer_puts(state, ":\n");
    } else {
        ls_printer_puts(state, this_dir->name);
        ls_printer_puts(state, ":\n");
    }
}

static void ls_print_total_header(struct ls_state* state, struct ls_dir* this_dir) {
    char buf[LS_NSTR_MAX+6];
    snprintf(buf, LS_NSTR_MAX+6, "total %lu", this_dir->blocks);
    if (state->options.format_type == 1) {
        ls_printer_putfs(state, LS_FORMAT_HEADER_TOTAL, buf);
        ls_printer_putr(state);
    } else {
        ls_printer_puts(state, buf);
    }
    ls_printer_puts(state, "\n");
}

static void ls_print_filename(struct ls_state* state, struct ls_file* fl) {
    struct ls_preproc_page* curpage = fl->pp_page;
    if (state->options.print_icons == 1) {
        if (state->pp_state.icon_w > 0) {
            if (curpage->format->icon != ls_icon_invalid) {
                if (state->options.format_type == 1) {
                    struct ls_color_iter* ifg = &(curpage->format->fg);
                    struct ls_color_iter* ibg = &(curpage->format->bg);
                    if (curpage->format->flags & LS_FORMAT_FLAG_HAS_ICON_COLOR) {
                        ifg = &(curpage->format->ifg);
                        ibg = &(curpage->format->ibg);
                    }
                    ls_printer_putcolor_s(
                        state,
                        ifg,
                        ibg,
                        curpage->format->icon,
                        strlen(curpage->format->icon)
                    );
                    ls_printer_puts(state, curpage->format->icon);
                    ls_printer_putr(state);
                    ls_printer_putch(state, ' ');
                } else {
                    ls_printer_puts(state, curpage->format->icon);
                }
            } else {
                ls_printer_puts(state, "  ");
            }
        }
    }
    if (state->options.format_type == 1) {
        size_t nlen = strlen(fl->name);
        if (curpage->format->flags & LS_FORMAT_FLAG_HAS_BORDER) {
            curpage->format->flags ^= LS_FORMAT_FLAG_HAS_BORDER;
            if (curpage->format->flags & LS_FORMAT_FLAG_HAS_BORDER_LEFT) {
                ls_printer_putf_c(
                    state,
                    curpage->format,
                    1
                );
                ls_printer_puts(
                    state,
                    curpage->format->border_left
                );
            }
            curpage->format->flags ^= LS_FORMAT_FLAG_HAS_BORDER;
        }
        ls_printer_putf_c(state, curpage->format, 0);
        if (fl->ext == fl->name) {
            ls_printer_putcolor_s(
                state,
                &(curpage->format->fg),
                &(curpage->format->bg),
                fl->name,
                nlen
            );
        } else {
            if (curpage->format->flags & LS_FORMAT_FLAG_HAS_EXT_COLOR) {
                ls_printer_putcolor_s(
                    state,
                    &(curpage->format->fg),
                    &(curpage->format->bg),
                    fl->name,
                    (fl->ext - fl->name)
                );
                ls_printer_putcolor_s(
                    state,
                    &(curpage->format->efg),
                    &(curpage->format->ebg),
                    fl->name + (fl->ext - fl->name),
                    nlen - (fl->ext - fl->name)
                );
            } else {
                ls_printer_putcolor_s(
                    state,
                    &(curpage->format->fg),
                    &(curpage->format->bg),
                    fl->name,
                    nlen
                );
            }
        }
        if (curpage->format->flags & LS_FORMAT_FLAG_HAS_BORDER) {
            curpage->format->flags ^= LS_FORMAT_FLAG_HAS_BORDER;
            if (curpage->format->flags & LS_FORMAT_FLAG_HAS_BORDER_RIGHT) {
                ls_printer_putf_c(
                    state,
                    curpage->format,
                    1
                );
                ls_printer_puts(
                    state,
                    curpage->format->border_right
                );
            }
            curpage->format->flags ^= LS_FORMAT_FLAG_HAS_BORDER;
        }
        ls_printer_putr(state);
    } else {
        ls_printer_puts(state, fl->name);
    }
    if (state->options.classify_type > 0) {
        if (state->options.format_type == 1) {
            ls_printer_putr(state);
            switch (fl->clschar[0]) {
                case '/':
                    ls_printer_putf(state,LS_FORMAT_CLASS_DIR);
                    break;
                case '@':
                    ls_printer_putf(state,LS_FORMAT_CLASS_LINK);
                    break;
                case '|':
                    ls_printer_putf(state,LS_FORMAT_CLASS_FIFO);
                    break;
                case '*':
                    ls_printer_putf(state,LS_FORMAT_CLASS_EXEC);
                    break;
                case '=':
                    ls_printer_putf(state,LS_FORMAT_CLASS_SOCK);
                    break;
                case '>':
                    ls_printer_putf(state,LS_FORMAT_CLASS_DOOR);
                    break;
                case '#':
                    ls_printer_putf(state,LS_FORMAT_CLASS_BLK);
                    break;
                case '%':
                    ls_printer_putf(state,LS_FORMAT_CLASS_CHR);
                    break;
            }
            ls_printer_putch(state, fl->clschar[0]);
            ls_printer_putr(state);
        } else {
            ls_printer_puts(state, &fl->clschar[0]);
        }
    }
}

static void ls_print_mode(struct ls_state* state, struct ls_file* fl) {
    if (state->options.format_type == 1) {
        char* modes = fl->pp_page->mode_s;
        switch (modes[0]) {
            case '-':
                ls_printer_putf(state, LS_FORMAT_MODE_TYPE_OFF);
                break;
            case 'l':
                ls_printer_putf(state, LS_FORMAT_MODE_TYPE_LINK);
                break;
            case 'd':
                ls_printer_putf(state, LS_FORMAT_MODE_TYPE_DIR);
                break;
            case 'c':
                ls_printer_putf(state, LS_FORMAT_MODE_TYPE_CHR);
                break;
            case 'b':
                ls_printer_putf(state, LS_FORMAT_MODE_TYPE_BLK);
                break;
            case 's':
                ls_printer_putf(state, LS_FORMAT_MODE_TYPE_SOCK);
                break;
            case 'p':
                ls_printer_putf(state, LS_FORMAT_MODE_TYPE_FIFO);
                break;
        }
        ls_printer_putch(state,modes[0]);
        ls_printer_putr(state);
        switch (modes[1]) {
            case '-':
                ls_printer_putf(state, LS_FORMAT_MODE_USER_R_OFF);
                break;
            case 'r':
                ls_printer_putf(state, LS_FORMAT_MODE_USER_R_ON);
                break;
        }
        ls_printer_putch(state,modes[1]);
        ls_printer_putr(state);
        switch (modes[2]) {
            case '-':
                ls_printer_putf(state, LS_FORMAT_MODE_USER_W_OFF);
                break;
            case 'w':
                ls_printer_putf(state, LS_FORMAT_MODE_USER_W_ON);
                break;
        }
        ls_printer_putch(state,modes[2]);
        ls_printer_putr(state);
        switch (modes[3]) {
            case '-':
                ls_printer_putf(state, LS_FORMAT_MODE_USER_X_OFF);
                break;
            case 'S':
                ls_printer_putf(state, LS_FORMAT_MODE_USER_X_OFF_SETUID);
                break;
            case 'x':
                ls_printer_putf(state, LS_FORMAT_MODE_USER_X_ON);
                break;
            case 's':
                ls_printer_putf(state, LS_FORMAT_MODE_USER_X_ON_SETUID);
                break;
        }
        ls_printer_putch(state,modes[3]);
        ls_printer_putr(state);
        switch (modes[4]) {
            case '-':
                ls_printer_putf(state, LS_FORMAT_MODE_GROUP_R_OFF);
                break;
            case 'r':
                ls_printer_putf(state, LS_FORMAT_MODE_GROUP_R_ON);
                break;
        }
        ls_printer_putch(state,modes[4]);
        ls_printer_putr(state);
        switch (modes[5]) {
            case '-':
                ls_printer_putf(state, LS_FORMAT_MODE_GROUP_W_OFF);
                break;
            case 'w':
                ls_printer_putf(state, LS_FORMAT_MODE_GROUP_W_ON);
                break;
        }
        ls_printer_putch(state,modes[5]);
        ls_printer_putr(state);
        switch (modes[6]) {
            case '-':
                ls_printer_putf(state, LS_FORMAT_MODE_GROUP_X_OFF);
                break;
            case 'S':
                ls_printer_putf(state, LS_FORMAT_MODE_GROUP_X_OFF_SETGID);
                break;
            case 'x':
                ls_printer_putf(state, LS_FORMAT_MODE_GROUP_X_ON);
                break;
            case 's':
                ls_printer_putf(state, LS_FORMAT_MODE_GROUP_X_ON_SETGID);
                break;
        }
        ls_printer_putch(state,modes[6]);
        ls_printer_putr(state);
        switch (modes[7]) {
            case '-':
                ls_printer_putf(state, LS_FORMAT_MODE_OTHER_R_OFF);
                break;
            case 'r':
                ls_printer_putf(state, LS_FORMAT_MODE_OTHER_R_ON);
                break;
        }
        ls_printer_putch(state,modes[7]);
        ls_printer_putr(state);
        switch (modes[8]) {
            case '-':
                ls_printer_putf(state, LS_FORMAT_MODE_OTHER_W_OFF);
                break;
            case 'w':
                ls_printer_putf(state, LS_FORMAT_MODE_OTHER_W_ON);
                break;
        }
        ls_printer_putch(state,modes[8]);
        ls_printer_putr(state);
        switch (modes[9]) {
            case '-':
                ls_printer_putf(state, LS_FORMAT_MODE_OTHER_X_OFF);
                break;
            case 'T':
                ls_printer_putf(state, LS_FORMAT_MODE_OTHER_X_OFF_STICKY);
                break;
            case 'x':
                ls_printer_putf(state, LS_FORMAT_MODE_OTHER_X_ON);
                break;
            case 't':
                ls_printer_putf(state, LS_FORMAT_MODE_OTHER_X_ON_STICKY);
                break;
        }
        ls_printer_putch(state,modes[9]);
        ls_printer_putr(state);
    } else {
        ls_printer_puts(state, fl->pp_page->mode_s);
    }
}

static void ls_print_nlinks(struct ls_state* state, struct ls_file* fl) {
    if (state->options.format_type == 1) {
        ls_printer_putfs(state, LS_FORMAT_NLINKS, fl->pp_page->nlinks_s);
        ls_printer_putr(state);
    } else {
        ls_printer_puts(state, fl->pp_page->nlinks_s);
    }
}

static void ls_print_uname(struct ls_state* state, struct ls_file* fl) {
    if (state->options.format_type == 1) {
        ls_printer_putfs(state, LS_FORMAT_OWNER, fl->pp_page->uname);
        ls_printer_putr(state);
    } else {
        ls_printer_puts(state, fl->pp_page->uname);
    }
}

static void ls_print_gname(struct ls_state* state, struct ls_file* fl) {
    if (state->options.format_type == 1) {
        ls_printer_putfs(state, LS_FORMAT_GROUP, fl->pp_page->gname);
        ls_printer_putr(state);
    } else {
        ls_printer_puts(state, fl->pp_page->gname);
    }
}

static void ls_print_size(struct ls_state* state, struct ls_file* fl) {
    if (state->options.format_type == 1) {
        ls_printer_putfs(state, LS_FORMAT_SIZE, fl->pp_page->size_s);
        ls_printer_putr(state);
        ls_printer_putfs(state, LS_FORMAT_SIZE_UNIT, fl->pp_page->size_units);
        ls_printer_putr(state);
    } else {
        ls_printer_puts(state, fl->pp_page->size_s);
        ls_printer_puts(state, fl->pp_page->size_units);
    }
}

static void ls_print_date(struct ls_state* state, struct ls_file* fl) {
    if (state->options.format_type == 1) {
        ls_printer_putfs(state, LS_FORMAT_DATE, fl->pp_page->date_s);
        ls_printer_putr(state);
    } else {
        ls_printer_puts(state, fl->pp_page->date_s);
    }
}

static void ls_print_inode(struct ls_state* state, struct ls_file* fl) {
    if (state->options.format_type == 1) {
        ls_printer_putfs(state, LS_FORMAT_INODE, fl->pp_page->inode_s);
        ls_printer_putr(state);
    } else {
        ls_printer_puts(state, fl->pp_page->inode_s);
    }
}

static void ls_print_blocks(struct ls_state* state, struct ls_file* fl) {
    if (state->options.format_type == 1) {
        ls_printer_putfs(state, LS_FORMAT_BLOCKS, fl->pp_page->blocks_s);
        ls_printer_putr(state);
    } else {
        ls_printer_puts(state, fl->pp_page->blocks_s);
    }
}

static void ls_print_link(struct ls_state* state, struct ls_file* fl) {
    if (state->options.format_type == 1) {
        ls_printer_putfs(state, LS_FORMAT_LINK_ARROW, " -> ");
        ls_printer_putr(state);
        ls_printer_putfs(state, LS_FORMAT_LINK_DEST, fl->link_to);
        ls_printer_putr(state);
    } else {
        ls_printer_puts(state, " -> ");
        ls_printer_puts(state, fl->link_to);
    }
}

static void ls_print_branch(struct ls_state* state, int has_child) {
    int b_idx = 0;
    if (state->options.format_type == 1) {
        ls_printer_putf(state, LS_FORMAT_TREE_BRANCH);
    }
    while (state->pr_state.tree_state[b_idx] != LS_PRINTER_BRANCH_NULL) {
        if (b_idx > 0) {
            ls_printer_puts(state, "  ");
        }
        switch (state->pr_state.tree_state[b_idx]) {
            case LS_PRINTER_BRANCH_NONE:
                ls_printer_puts(state, " ");
                break;
            case LS_PRINTER_BRANCH_TURN:
                ls_printer_puts(state, "└");
                break;
            case LS_PRINTER_BRANCH_LEAF:
                ls_printer_puts(state, "├");
                break;
            case LS_PRINTER_BRANCH_DOWN:
                ls_printer_puts(state, "│");
                break;
            default:
                break;
        }
        b_idx++;
    }
    ls_printer_puts(state, "──");
    if (has_child) {
        ls_printer_puts(state, "┬");
    } else {
        ls_printer_puts(state, "─");
    }
    if (state->options.format_type == 1) {
        ls_printer_putr(state);
    }
    ls_printer_putch(state, ' ');
}

#if LS_BUILD_GIT

static void ls_print_git_status(struct ls_state* state, char* status) {
    if (state->options.format_type == 1) {
        switch (status[0]) {
            case '-':
                ls_printer_putf(state, LS_FORMAT_GIT_STATUS_INDEX_NONE);
                break;
            case '?':
                ls_printer_putf(state, LS_FORMAT_GIT_STATUS_INDEX_UNKNOWN);
                break;
            case 'A':
                ls_printer_putf(state, LS_FORMAT_GIT_STATUS_INDEX_NEW);
                break;
            case 'M':
                ls_printer_putf(state, LS_FORMAT_GIT_STATUS_INDEX_MODIFIED);
                break;
            case 'D':
                ls_printer_putf(state, LS_FORMAT_GIT_STATUS_INDEX_DELETED);
                break;
            case 'R':
                ls_printer_putf(state, LS_FORMAT_GIT_STATUS_INDEX_RENAMED);
                break;
            case 'T':
                ls_printer_putf(state, LS_FORMAT_GIT_STATUS_INDEX_TYPECHANGE);
                break;
            case '!':
                ls_printer_putf(state, LS_FORMAT_GIT_STATUS_INDEX_IGNORED);
                break;
        }
        ls_printer_putch(state, status[0]);
        switch (status[1]) {
            case '-':
                ls_printer_putf(state, LS_FORMAT_GIT_STATUS_WT_NONE);
                break;
            case '?':
                ls_printer_putf(state, LS_FORMAT_GIT_STATUS_WT_UNKNOWN);
                break;
            case 'A':
                ls_printer_putf(state, LS_FORMAT_GIT_STATUS_WT_NEW);
                break;
            case 'M':
                ls_printer_putf(state, LS_FORMAT_GIT_STATUS_WT_MODIFIED);
                break;
            case 'D':
                ls_printer_putf(state, LS_FORMAT_GIT_STATUS_WT_DELETED);
                break;
            case 'R':
                ls_printer_putf(state, LS_FORMAT_GIT_STATUS_WT_RENAMED);
                break;
            case 'T':
                ls_printer_putf(state, LS_FORMAT_GIT_STATUS_WT_TYPECHANGE);
                break;
            case '!':
                ls_printer_putf(state, LS_FORMAT_GIT_STATUS_WT_IGNORED);
                break;
        }
        ls_printer_putch(state, status[1]);
    } else {
        ls_printer_putch(state, status[0]);
        ls_printer_putch(state, status[1]);
    }
    ls_printer_putch(state, ' ');
}

#endif

void ls_print_files(struct ls_state* state, struct ls_dir* this_dir, int rlvl) {
    int listing_type = state->options.listing_type;
    if (rlvl == 0) {
        if (state->options.print_headers) {
            ls_print_dirname_header(state,this_dir);
        }
        if (state->options.print_blocks) {
            ls_print_total_header(state,this_dir);
        }
    }
    if (listing_type == LS_LISTING_LONG) {
        for (unsigned int i=0;i<this_dir->count;i++) {
            struct ls_file* curfile = this_dir->files_ptr[i];
            // Print inode, if applicable, justified right
            if (state->options.print_inodes) {
                ls_printer_puts_justify(
                    state, 
                    curfile->pp_page->inode_s, 
                    state->pp_state.inode_w
                );
                ls_print_inode(state, curfile);
                ls_printer_putch(state, ' ');
            }
            // Print blocks, if applicable, justified right
            if (state->options.print_block_numbers) {
                ls_printer_puts_justify(
                    state, 
                    curfile->pp_page->blocks_s, 
                    state->pp_state.blocks_w
                );
                ls_print_blocks(state, curfile);
                ls_printer_putch(state, ' ');
            }
            // Print mode
            ls_print_mode(state, curfile);
            ls_printer_putch(state, ' ');
            // Print number of links, justified right (?)
            ls_printer_puts_justify(
                state, 
                curfile->pp_page->nlinks_s, 
                state->pp_state.nlinks_w
            );
            ls_print_nlinks(state, curfile);
            ls_printer_putch(state, ' ');
            // Print username, justified left
            if (state->options.print_owner) {
                ls_print_uname(state, curfile);
                ls_printer_puts_justify(
                    state, 
                    curfile->pp_page->uname, 
                    state->pp_state.uname_w
                );
                ls_printer_putch(state, ' ');
            }
            // Print group name (or number), justified left
            if (state->options.print_group) {
                ls_print_gname(state, curfile);
                ls_printer_puts_justify(
                    state, 
                    curfile->pp_page->gname, 
                    state->pp_state.gname_w
                );
                ls_printer_putch(state, ' ');
            }
            // Print size, justified right
            ls_printer_puts_justify(
                state, 
                curfile->pp_page->size_s, 
                state->pp_state.size_w-strlen(curfile->pp_page->size_units)
            );
            ls_print_size(state, curfile);
            ls_printer_putch(state, ' ');
            // Print date, justified left
            ls_print_date(state, curfile);
            ls_printer_puts_justify(
                state, 
                curfile->pp_page->date_s, 
                state->pp_state.date_w
            );
            ls_printer_putch(state, ' ');
#if LS_BUILD_GIT
            if (state->options.git_open) {
                if (state->pp_state.gs_w > 0) {
                    ls_print_git_status(state, curfile->pp_page->git_status_s);
                }
            }
#endif
            // Print filename
            if (state->options.listing_tree) {
                if (curfile->pp_page->ilvl > -1) {
                    int t_idx = curfile->pp_page->ilvl;
                    if (i == this_dir->count - 1) {
                        state->pr_state.tree_state[t_idx] = LS_PRINTER_BRANCH_TURN;
                    } else {
                        state->pr_state.tree_state[t_idx] = LS_PRINTER_BRANCH_LEAF;
                    }
                    if (curfile->child == NULL) {
                        ls_print_branch(state, 0);
                    } else {
                        if (curfile->child->count) {
                            ls_print_branch(state, 1);
                        } else {
                            ls_print_branch(state, 0);
                        }
                    }
                }
            }
            ls_print_filename(state, curfile);
            // If the file is a link, print where it goes to
            if (S_ISLNK(curfile->status.st_mode)) {
                ls_print_link(state, curfile);
            }
            // If listing is tree and there are more files, indicate that
            if (state->options.listing_tree) {
                if ((i == this_dir->count - 1) && (rlvl != 0) && (this_dir->count != this_dir->max)) {
                    char buf[LS_NSTR_MAX+6];
                    snprintf(buf, LS_NSTR_MAX+6, "%lu", (this_dir->max - this_dir->count));
                    if (state->options.format_type == 1) {
                        ls_printer_putfs(state, LS_FORMAT_TREE_MORE_PAREN, " (... ");
                        ls_printer_putr(state);
                        ls_printer_putfs(state, LS_FORMAT_TREE_MORE_COUNT, buf);
                        ls_printer_putr(state);
                        ls_printer_putfs(state, LS_FORMAT_TREE_MORE_PAREN, " more)");
                        ls_printer_putr(state);
                    } else {
                        ls_printer_puts(state, " (... ");
                        ls_printer_puts(state, buf);
                        ls_printer_puts(state, " more)");
                    }
                }
            }
            ls_printer_puts(state, "\n");
            if (state->options.listing_tree) {
                int t_idx = curfile->pp_page->ilvl;
                if (curfile->child != NULL) {
                    if (t_idx >= 0) {
                        if (i == this_dir->count - 1) {
                            state->pr_state.tree_state[t_idx] = LS_PRINTER_BRANCH_NONE;
                        } else {
                            state->pr_state.tree_state[t_idx] = LS_PRINTER_BRANCH_DOWN;
                        }
                    }
                    ls_print_files(state, curfile->child, rlvl+1);
                }
                if (t_idx >= 0) {
                    if (i == this_dir->count - 1) {
                        state->pr_state.tree_state[t_idx] = LS_PRINTER_BRANCH_NULL;
                    }
                }
            }
        }
    } else if (listing_type == LS_LISTING_STREAM) {
        for (unsigned int i=0;i<this_dir->count;i++) {
            struct ls_file* curfile = this_dir->files_ptr[i];
            // Print inode, if applicable
            if (state->options.print_inodes) {
                ls_print_inode(state, curfile);
                ls_printer_putch(state, ' ');
            }
            // Print filename
            ls_print_filename(state, curfile);
            if (i != (this_dir->count-1)) {
                ls_printer_puts(state, ", ");
            }
        }
    } else if (listing_type == LS_LISTING_REGULAR) {
        for (unsigned int i=0;i<this_dir->count;i++) {
            struct ls_file* curfile = this_dir->files_ptr[i];
            // Print inode, if applicable, justified right
            if (state->options.print_inodes) {
                ls_printer_puts_justify(
                    state,
                    curfile->pp_page->inode_s, 
                    state->pp_state.inode_w
                );
                ls_print_inode(state, curfile);
                ls_printer_putch(state, ' ');
            }
            ls_print_filename(state, curfile);
            if (state->options.format_type == 1) {
                ls_printer_putr(state);
            }
            ls_printer_putln(state);
        }
    } else if (listing_type == LS_LISTING_COLUMNS) {
        int numlines;
        if (this_dir->count == 0) {
            numlines = 0;
        } else {
            numlines = CEIL_DIVIDE((this_dir->count),(state->pp_state.ncols));
        }
        for (int ri=0;ri<numlines;ri++) {
            for (int ci=0;ci<state->pp_state.ncols;ci++) {
                unsigned int fi=(ci*numlines)+ri;
                if (fi < this_dir->count) {
                    struct ls_file* curfile = this_dir->files_ptr[fi];
                    ls_print_filename(state, curfile);
                    int spc = 0;
                    if (state->options.classify_type) {
                        if (curfile->clschar[0] == '\0') {
                            spc = 1;
                        }
                    }
                    if (fi != this_dir->count - 1) {
                        ls_printer_puts_justify_l(
                            state,
                            curfile->pp_page->name_l,
                            state->pp_state.cols_w[ci]+spc
                        );
                    }
                }
            }
            ls_printer_putln(state);
        }
    } else if (listing_type == LS_LISTING_ROWS) {
        int numlines;
        if (this_dir->count == 0) {
            numlines = 0;
        } else {
            numlines = CEIL_DIVIDE((this_dir->count),(state->pp_state.ncols));
        }
        for (int ri=0;ri<numlines;ri++) {
            for (int ci=0;ci<state->pp_state.ncols;ci++) {
                unsigned int fi=(ri*state->pp_state.ncols)+ci;
                if (fi < this_dir->count) {
                    struct ls_file* curfile = this_dir->files_ptr[fi];
                    ls_print_filename(state, curfile);
                    int spc = 0;
                    if (state->options.classify_type) {
                        if (curfile->clschar[0] == '\0') {
                            spc = 1;
                        }
                    }
                    if (fi != this_dir->count - 1) {
                        ls_printer_puts_justify_l(
                            state,
                            curfile->pp_page->name_l,
                            state->pp_state.cols_w[ci]+spc
                        );
                    }
                }
            }
            ls_printer_putln(state);
        }
    }
}