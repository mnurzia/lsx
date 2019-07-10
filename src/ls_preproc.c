#include "ls_preproc.h"

#if LS_BUILD_GIT

#include "git2.h"

#endif

#include "ls_state.h"

const char* ls_units_regular[9] =   {"", "K", "M", "G", "T", "P", "E", "Z", "Y"};
const char* ls_units_si[9] =        {"", "k", "M", "G", "T", "P", "E", "Z", "Y"};
const char* ls_units_canon[9] =     {"B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};

static size_t ls_code_points(const char *s) {
    size_t count = 0;
    while (*s) {
        count += (*s++ & 0xC0) != 0x80;
    }
    return count;
}

static char ls_preproc_classify_file(struct ls_state* state, struct ls_file* fl) {
    mode_t md = fl->status.st_mode;
    if (state->options.classify_type == LS_CLASSIFY_DIRECTORIES) {
        if (S_ISDIR(md)) {
            return '/';
        } else {
            return '\0';
        }
    } else {
        if (S_ISDIR(md)) {
            return '/';
        } else if (S_ISLNK(md)) {
            return '@';
        } else if (S_ISFIFO(md)) {
            return '|';
        } else if (S_ISSOCK(md)) {
            return '=';
        } else if (S_ISBLK(md)) {
            return '#';
        } else if (S_ISCHR(md)) {
            return '%';
        } else if ((md & S_IXUSR) || (md & S_IXGRP) || (md & S_IXOTH)) {
            return '*';
        } else {
            return '\0';
        }
    }
}

static void ls_preproc_strmode(mode_t md, char* out) {
    switch (md & S_IFMT){
    default:
        out[0] = '-';
        break;
    case S_IFLNK:
        out[0] = 'l';
        break;
    case S_IFDIR:
        out[0] = 'd';
        break;
    case S_IFCHR:
        out[0] = 'c';
        break;
    case S_IFBLK:
        out[0] = 'b';
        break;
    case S_IFSOCK:
        out[0] = 's';
        break;
    case S_IFIFO:
        out[0] = 'p';
        break;
    }
    out[1] = (md & S_IRUSR) ? 'r' : '-';
    out[2] = (md & S_IWUSR) ? 'w' : '-';
    if (md & S_IXUSR) {
        if (md & S_ISUID) {
            out[3] = 's';
        } else {
            out[3] = 'x';
        }
    } else {
        if (md & S_ISUID) {
            out[3] = 'S';
        } else {
            out[3] = '-';
        }
    }
    out[4] = (md & S_IRGRP) ? 'r' : '-';
    out[5] = (md & S_IWGRP) ? 'w' : '-';
    if (md & S_IXGRP) {
        if (md & S_ISGID) {
            out[6] = 's';
        } else {
            out[6] = 'x';
        }
    } else {
        if (md & S_ISGID) {
            out[6] = 'S';
        } else {
            out[6] = '-';
        }
    }
    out[7] = (md & S_IROTH) ? 'r' : '-';
    out[8] = (md & S_IWOTH) ? 'w' : '-';
    if (md & S_IXOTH) {
        if (S_ISDIR(md)) {
            if (md & S_ISVTX) {
                out[9] = 't';
            } else {
                out[9] = 'x';
            }
        } else {
            out[9] = 'x';
        }
    } else {
        if (S_ISDIR(md)) {
            if (md & S_ISVTX) {
                out[9] = 'T';
            } else {
                out[9] = 'x';
            }
        } else {
            out[9] = 'x';
        }
    }
    out[10] = '\0';
    out[11] = '\0';
}

#if LS_BUILD_GIT

static void ls_preproc_git_status(git_status_t gs, char* out) {
    out[0] = '-';
    out[1] = '-';
    out[2] = '\0';
    if (gs & GIT_STATUS_INDEX_NEW) {
        out[0] = 'A';
    }
    if (gs & GIT_STATUS_INDEX_MODIFIED) {
        out[0] = 'M';
    }
    if (gs & GIT_STATUS_INDEX_DELETED) {
        out[0] = 'D';
    }
    if (gs & GIT_STATUS_INDEX_RENAMED) {
        out[0] = 'R';
    }
    if (gs & GIT_STATUS_INDEX_TYPECHANGE) {
        out[0] = 'T';
    }
    if (gs & GIT_STATUS_WT_NEW) {
        if (out[1] == '-') {
            out[0] = '?';
        }
        out[1] = '?';
    }
    if (gs & GIT_STATUS_WT_MODIFIED) {
        out[1] = 'M';
    }
    if (gs & GIT_STATUS_WT_DELETED) {
        out[1] = 'D';
    }
    if (gs & GIT_STATUS_WT_RENAMED) {
        out[1] = 'R';
    }
    if (gs & GIT_STATUS_WT_TYPECHANGE) {
        out[1] = 'T';
    }
    
    if (gs & GIT_STATUS_IGNORED) {
        out[0] = out[1] = '!';
    }
}

#endif

void ls_preproc_files(struct ls_state* state, struct ls_dir* this_dir) {
    // Preprocessing the data makes it so that the numbers in different views
    // (e.g. long mode) are padded and aligned nicely in the terminal.
    // It usually requires stat_files to be set to 1.
    struct ls_preproc_state* pp_state = &(state->pp_state);
    pp_state->count = 0;
    pp_state->nlinks_w = 0;
    pp_state->uname_w = 0;
    pp_state->gname_w = 0;
    pp_state->size_w = 0;
    pp_state->date_w = 0;
    pp_state->inode_w = 0;
    pp_state->name_w = 0;
    pp_state->blocks_w = 0;
    pp_state->icon_w = 0;
#if LS_BUILD_GIT
    pp_state->gs_w = 0;
#endif
    // If the table is not initialized, do that
    if (pp_state->initialized == 0) {
        pp_state->allocsize = 16;
        pp_state->pages = malloc(sizeof(struct ls_preproc_page)*pp_state->allocsize);
        if (pp_state->pages == NULL) {
            ls_do_error(state, "Couldn't allocate space for preprocessor pages");
        }
        pp_state->cols_w = malloc(sizeof(int)*state->options.ncols);
        time(&pp_state->now);
        pp_state->initialized = 1;
    }
    // Find which fields to evaluate.
    int listing_type = state->options.listing_type;
    int do_nlinks = 0;
    int do_uname = 0;
    int do_gname = 0;
    int do_size = 0;
    int do_date = 0;
    int do_mode = 0;
    int do_inode = 0;
    int do_columns = 0;
    int do_rows = 0;
    int do_name = 0;
    int do_blocks = 0;
    int do_icons = 0;
    int do_colors = 0;
#if LS_BUILD_GIT
    int do_git = 0;
#endif
    if (listing_type == LS_LISTING_LONG) {
        do_nlinks = 1;
        do_uname = 1;
        do_gname = 1;
        do_size = 1;
        do_date = 1;
        do_mode = 1;
    }
    if (listing_type == LS_LISTING_COLUMNS) {
        do_columns = 1;
        do_name = 1;
    }
    if (listing_type == LS_LISTING_ROWS) {
        do_rows = 1;
        do_name = 1;
    }
    if (state->options.print_inodes) {
        do_inode = 1;
    }
    if (state->options.print_block_numbers) {
        do_blocks = 1;
    }
    if (state->options.print_icons) {
        do_icons = 1;
    }
    if (state->options.format_type == 1) {
        do_colors = 1;
    }
#if LS_BUILD_GIT
    if (state->options.git_open) {
        do_git = 1;
    }
#endif
    size_t count_full = 0;
    int done_listing = 0;
    struct ls_dir* dir_stack[17];
    size_t count_stack[17];
    size_t stack_ptr = 0;
    dir_stack[stack_ptr] = this_dir;
    count_stack[stack_ptr] = 0;
    // If there isn't enough space in the table, allocate some
    if (this_dir->count_full >= pp_state->allocsize) {
        pp_state->allocsize = ((this_dir->count_full) + (this_dir->count_full>>2));
        pp_state->pages = realloc(pp_state->pages, sizeof(struct ls_preproc_page)*pp_state->allocsize);
        if (pp_state->pages == NULL) {
            ls_do_error(state, "Couldn't reallocate space for preprocessor pages");
        }
    }
    while (!done_listing) {
        struct ls_dir* cur_dir = dir_stack[stack_ptr];
        int i = count_stack[stack_ptr];
        if (!(count_stack[stack_ptr] < dir_stack[stack_ptr]->count)) {
            if (stack_ptr == 0) {
                done_listing = 1;
                continue;
            } else {
                stack_ptr--;
                continue;
            }
        }
        struct ls_file* curfile = cur_dir->files_ptr[i]; 
        curfile->pp_page = &(pp_state->pages[count_full]);
        struct ls_preproc_page* curpage = curfile->pp_page;
        curpage->ilvl = stack_ptr;
        if (do_nlinks) {
            // Record the width of the number of lengths string, also generate
            // that string and store it
            unsigned int tsz = snprintf(
                curpage->nlinks_s,
                LS_NSTR_MAX,
                "%lu",
                (long unsigned int)curfile->status.st_nlink
            );
            pp_state->nlinks_w = (tsz > pp_state->nlinks_w) ? tsz : pp_state->nlinks_w;
        }
        if (do_size) {
            // Record the width of the size string, also generate that string
            // and store it
            unsigned int tsz = 0;
            if (state->options.size_type == LS_SIZE_REGULAR) {
                tsz = snprintf(
                    curpage->size_s,
                    LS_NSTR_MAX,
                    "%li",
                    curfile->status.st_size
                );
                curpage->size_units = "";
            } else {
                int unit_idx = 0;
                double im = (double)curfile->status.st_size;
                while (im > state->options.size_power) {
                    im /= state->options.size_power;
                    unit_idx++;
                }
                if (state->options.size_type == LS_SIZE_HUMAN_READABLE) {
                    tsz = sprintf(
                        curpage->size_s,
                        "%.0f",
                        ceil(im)
                    );
                } else if (state->options.size_type == LS_SIZE_HUMAN_READABLE_FLOAT) {
                    tsz = sprintf(
                        curpage->size_s,
                        "%.*f",
                        unit_idx,
                        ceil(im)
                    );
                }
                curpage->size_units = (char*)state->options.size_units[unit_idx];
                tsz += strlen(curpage->size_units);
            }
            pp_state->size_w = (tsz > pp_state->size_w) ? tsz : pp_state->size_w;
        }
        if (do_date) {
            // Record the date for each file. The standard dictates that if the
            // file has not been modified for six months, to use a different
            // format for the date.
            struct tm* tmp;
            double time_diff;
            double six_months = 60*60*24*30*6;
            unsigned int tsz;
            time_t tm_to_use;
            if (state->options.date_type == LS_DATE_MTIME) {
                tm_to_use = curfile->status.st_mtime;
            } else if (state->options.date_type == LS_DATE_CTIME) {
                tm_to_use = curfile->status.st_ctime;
            } else if (state->options.date_type == LS_DATE_ATIME) {
                tm_to_use = curfile->status.st_atime;
            } else {
                tm_to_use = curfile->status.st_mtime;
            }
            time_diff = difftime(pp_state->now, tm_to_use);
            tmp = localtime(&tm_to_use);
            if (time_diff < six_months) {
                tsz = strftime(
                    curpage->date_s,
                    LS_DSTR_MAX,
                    "%b %e %H:%M",
                    tmp
                );
            } else {
                tsz = strftime(
                    curpage->date_s,
                    LS_DSTR_MAX,
                    "%b %e  %Y",
                    tmp
                );
            }
            pp_state->date_w = (tsz > pp_state->date_w) ? tsz : pp_state->date_w;
        }
        if (do_uname) {
            // Record the width of the username string for each file. Also
            // assign it to the file's page.
            struct ls_uid_cache_page* cuid;
            cuid = ls_uidtbl_get(
                state, 
                curfile->status.st_uid
            );
            curpage->uname = cuid->name;
            pp_state->uname_w = (cuid->size > pp_state->uname_w) ? cuid->size : pp_state->uname_w;
        }
        if (do_gname) {
            // Record the width of the group string for each file. Also
            // assign it to the file's page.
            struct ls_gid_cache_page* cgid;
            cgid = ls_gidtbl_get(
                state, 
                curfile->status.st_gid
            );
            curpage->gname = cgid->name;
            pp_state->gname_w = (cgid->size > pp_state->gname_w) ? cgid->size : pp_state->gname_w;
        }
        if (do_mode) {
            // Record the mode for each file.
            ls_preproc_strmode(curfile->status.st_mode,curpage->mode_s);
        }
        if (do_inode) {
            // Record the inode (serial number) for each file.
            unsigned int tsz = snprintf(
                curpage->inode_s,
                LS_NSTR_MAX,
                "%lu",
                curfile->status.st_ino
            );
            pp_state->inode_w = (tsz > pp_state->inode_w) ? tsz : pp_state->inode_w;
        }
        if (do_blocks) {
            // Record the number of blocks for each file
            unsigned int tsz = snprintf(
                curpage->blocks_s,
                LS_NSTR_MAX,
                "%lu",
                curfile->status.st_blocks
            );
            pp_state->blocks_w = (tsz > pp_state->blocks_w) ? tsz : pp_state->blocks_w;
        }
        if (state->options.classify_type > 0) {
            curfile->clschar[0] = ls_preproc_classify_file(state, curfile);
            curfile->clschar[1] = '\0';
        }
        if (do_colors || do_icons) {
            curpage->format = NULL;
            struct ls_file* fl = curfile;
            struct ls_format* form = NULL;
            HASH_FIND_STR(state->theme.extensions, fl->name, form);
            if (form == NULL) {
                HASH_FIND_STR(state->theme.extensions, fl->ext, form);
            }
            if (form != NULL) {
                if (do_icons) {
                    if (form->icon != ls_icon_invalid) {
                        pp_state->icon_w = 2;
                    }
                }
                curpage->format = form;
            } else {
                int fmt = LS_FORMAT_OBJ_NORMAL;
                mode_t md = fl->status.st_mode;
                if (S_ISREG(md)) {
                    // As of 5/11/2019, this works. Don't mess around!
                    fmt = LS_FORMAT_OBJ_FILE;
                    if ((md & S_ISUID) != 0 && ls_theme_has_format(state,LS_FORMAT_OBJ_SETUID)) {
                        fmt = LS_FORMAT_OBJ_SETUID;
                    } else if ((md & S_ISGID) != 0 && ls_theme_has_format(state,LS_FORMAT_OBJ_SETGID)) {
                        fmt = LS_FORMAT_OBJ_SETGID;
                    } else if (
                        ((md & S_IXGRP) | 
                         (md & S_IXUSR) | 
                         (md & S_IXOTH)) != 0 && 
                         ls_theme_has_format(state,LS_FORMAT_OBJ_EXEC)) {
                        fmt = LS_FORMAT_OBJ_EXEC;
                    } else if ((fl->status.st_nlink > 1) && ls_theme_has_format(state,LS_FORMAT_OBJ_MULTI_HARD_LINK)) {
                        fmt = LS_FORMAT_OBJ_MULTI_HARD_LINK; 
                    }
                } else if (S_ISDIR(md)) {
                    fmt = LS_FORMAT_OBJ_DIR;
                    if ((md & S_ISVTX) && (md & S_IWOTH)
                        && ls_theme_has_format(state,LS_FORMAT_OBJ_STICKY_OTHER_WRITE)) {
                        fmt = LS_FORMAT_OBJ_STICKY_OTHER_WRITE;
                    } else if ((md & S_IWOTH) != 0 && ls_theme_has_format(state,LS_FORMAT_OBJ_OTHER_WRITE)) {
                        fmt = LS_FORMAT_OBJ_OTHER_WRITE;
                    } else if ((md & S_ISVTX) != 0 && ls_theme_has_format(state,LS_FORMAT_OBJ_STICKY)) {
                        fmt = LS_FORMAT_OBJ_STICKY;
                    }
                } else if (S_ISLNK(md)) {
                    fmt = LS_FORMAT_OBJ_LINK;
                } else if (S_ISFIFO(md)) {
                    fmt = LS_FORMAT_OBJ_FIFO;
                } else if (S_ISSOCK(md)) {
                    fmt = LS_FORMAT_OBJ_SOCK;
                } else if (S_ISBLK(md)) {
                    fmt = LS_FORMAT_OBJ_BLK;
                } else if (S_ISCHR(md)) {
                    fmt = LS_FORMAT_OBJ_CHR;
                }
                curpage->format = &(state->theme.formats[fmt]);
                if (curpage->format->icon != ls_icon_invalid) {
                    pp_state->icon_w = 2;
                }
            }
        }
        pp_state->count++;
        if (do_name) {
            unsigned int tsz = ls_code_points(curfile->name);
            if (do_colors) {
                if (curpage->format) {
                    if (curpage->format->flags & LS_FORMAT_FLAG_HAS_BORDER) {
                        if (curpage->format->flags & LS_FORMAT_FLAG_HAS_BORDER_LEFT) {
                            tsz += ls_code_points(curpage->format->border_left);
                        }
                        if (curpage->format->flags & LS_FORMAT_FLAG_HAS_BORDER_LEFT) {
                            tsz += ls_code_points(curpage->format->border_right);
                        }
                    }
                }
            }
            curpage->name_l = tsz;
            pp_state->name_w = (tsz > pp_state->name_w) ? tsz : pp_state->name_w;
        }
#if LS_BUILD_GIT
        if (do_git) {
            unsigned int tsz = 0;
            ls_preproc_git_status(curfile->git_status, curpage->git_status_s);
            if (!(curpage->git_status_s[0] == '-' && curpage->git_status_s[1] == '-')) {
                tsz = 2;
            }
            pp_state->gs_w = (tsz > pp_state->gs_w) ? tsz : pp_state->gs_w;
        }
#endif
        count_stack[stack_ptr]++;
        count_full++;
        if (curfile->child != NULL) {
            struct ls_dir* new_child = curfile->child;
            stack_ptr++;
            dir_stack[stack_ptr] = new_child;
            count_stack[stack_ptr] = 0;
        }
    }
    if (pp_state->name_w > state->options.ncols) {
        // If the max name width exceeds the max number of cols,
        // give up and switch to regular mode.
        // Not doing this would introduce a bit of undefined behavior.
        do_columns = 0;
        do_rows = 0;
        state->options.listing_type = LS_LISTING_REGULAR;
    }
    // 7/3/2018: I don't fucking know what to say here. This function sucks, it
    // took me a long-ass time to find, and it barely works. First I tried to make
    // my own routine for this. Except, I had no idea how to do it as well as
    // fuckin RMS did in ls.c. And examining that coreutils code was a dead end
    // because it was more cryptic than it was worth. How could something so
    // obfuscated produce such a clean-fitting, perfectly aligned result? -Cx
    // basically remained a wild goose chase since like April when I started
    // working on this. RMS bested me yet again, all hail GNU+linux, yada yada.
    // I could write something like exa's --grid which would work for 99.9% of
    // people, but I wanted a columnator that was 'smart' like GNU ls's. So I
    // left this alone, and actually took a break from May-June. Enough was
    // enough, I decided on 6/27. I got cracking. Yeah, I tried to write my own
    // version again but it didn't work out because I yet again couldn't figure
    // out the algorithm. Google was pretty much a no-go, maybe it was my search
    // query or maybe it just wasn't on the first 5 pages. There were a few
    // StackOverflow links sprinkled into the results, but I checked all of
    // them a few times, hoping to find some old forgotten words, or ancient
    // column algorithms. Picture me late at night, like 11:30pm, having let
    // this problem defeat me too many times, wondering why I was spending the
    // best days of my high school summer vacation figuring out how to print
    // column numbers nicely in the terminal. One last google search, I thought
    // to myself, couldn't hurt. And so one last google search I did. It came up
    // with all the usual links I was finding. I clicked on them anyways, hoping
    // that maybe I missed something earlier, maybe there was something I wasn't
    // getting. I was worried that if I didn't find something now, I might have to
    // *shudders* ask my own stackoverflow question. Anyways, there I'm browsing,
    // when I'm looking at some bullshit SO post and something catches me out of
    // the corner of my eye, i.e. the "related" section of the page. What's this?
    // "How can I calculate optimal column widths?" That sounds like EXACTLY WHAT
    // I'M TRYING TO DO. I click it, and what do you know, that's the answer.
    // This answers what I've been trying to do for too long. Oh man, this is
    // great. I even understood how the code worked (the underlying "algorithm")
    // and implemented it in like 45 minutes with a bit of gdb help here and there.
    // And it worked! I promptly fell asleep.
    // Next afternoon, I come back to the code. WHAT DID I WRITE?? How am I
    // supposed to understand this? Cue me running this against GNU ls, and
    // nearly shitting my pants after it came up with the exact same layout.
    // Now I'm no believer in God, but I can safely say that if he exists only
    // he knows how this function works. Moral of the story: The "Related"
    // section of StackOverflow is your friend. Also, man, RMS must have been on
    // something due to figuring this algorithm out a good 10+ years before I got
    // to it. (he probably was) Second moral of the story: sometimes, if your code
    // sucks but still seems to work fine, you're allowed to leave it that way.
    // Thank you for reading, without further ado, here is the algorithm:
    if (do_columns) {
        // Make the columns for each dir. <-- 7/3: what kind of comment is this? l'mao
        for (unsigned int c=this_dir->count; c>0; c--) {
            int max_dist = state->options.ncols;
            int entries_per_col = CEIL_DIVIDE(this_dir->count,c);
            for (unsigned int i=0;i<c;i++) {
                int col_width = 0;
                for (unsigned int j=i*entries_per_col;j<(i+1)*entries_per_col;j++) {
                    if (j < this_dir->count) {
                        int spc = 2;
                        col_width = MAX(this_dir->files_ptr[j]->pp_page->name_l+spc, col_width);
                    }
                }
                max_dist -= col_width;
                if (i < state->options.ncols) {
                    pp_state->cols_w[i] = col_width;
                }
            }
            if (max_dist >= 0) {
                pp_state->ncols = c;
                break;
            }
        }
    }
    if (do_rows) {
        // Make the columns for each dir, but in a row form 
        for (unsigned int c=this_dir->count; c>0; c--) {
            int max_dist = state->options.ncols;
            for (unsigned int i=0;i<c;i++) {
                int col_width = 0;
                for (unsigned int j=i;j<this_dir->count;j+=c) {
                    if (j < this_dir->count) {
                        int spc = 2;
                        col_width = MAX(this_dir->files_ptr[j]->pp_page->name_l+spc, col_width);
                    }
                }
                max_dist -= col_width;
                if (i < state->options.ncols) {
                    pp_state->cols_w[i] = col_width;
                }
            }
            if (max_dist >= 0) {
                pp_state->ncols = c;
                break;
            }
        }
    }
    this_dir->count_full = count_full;
    // Since you're here, here is the StackOverflow link to the algorithm I found:
    // https://stackoverflow.com/questions/4357275/how-can-i-calculate-optimal-column-widths
}