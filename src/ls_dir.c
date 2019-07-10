#include "ls_dir.h"

#include "stretchy_buffer.h"

#include "ls_state.h"

// Helper function to sanitize the paths of any cmdline args.
static void ls_dir_sanitize_path(char* pth) {
    size_t sz = strlen(pth);
    if (pth[sz-1] != '/') {
        strcat(pth, "/");
    }
}

// Makes a dir object
struct ls_dir* ls_dir_make(struct ls_state* state, char* name, int cmdline, ino_t inode) {
    struct ls_dir* curdir = malloc(sizeof(struct ls_dir));
    if (curdir == NULL) {
        ls_do_error(state, "Error allocating directory object");
    }
    // Copy the name to the new directory entry.
    // 2 extra bytes are allocated, one for the null terminator, and one
    // for another slash should it be required.
    curdir->name = malloc(sizeof(char)*(strlen(name)+2));
    if (curdir->name == NULL) {
        ls_do_error(state, "Error allocating directory name");
    }
    strcpy(curdir->name,name);
    curdir->cmdline = cmdline;
    if (curdir->cmdline) {
        curdir->cmdline_name = malloc(sizeof(char)*(strlen(name)+2));
        if (curdir->cmdline_name == NULL) {
            ls_do_error(state, "Error allocating directory command line name");
        }
        strcpy(curdir->cmdline_name,name);
        struct stat temp_stat;
        stat(curdir->name, &temp_stat);
        curdir->id = temp_stat.st_ino;
    } else {
        curdir->id = inode;
        curdir->cmdline_name = curdir->name;
    }
    ls_dir_sanitize_path(curdir->name);
    curdir->count = 0;
    curdir->count_full = 0;
    curdir->max = 0;
    curdir->files_ptr = NULL;
    curdir->tbl.count = 0;
    curdir->tbl.children_ptr = NULL;
#if LS_BUILD_GIT
    curdir->repo = NULL;
#endif
    return curdir;
}

// This is the function that actually does the listing part of ls. It uses 
// opendir and readdir to extract file/directory info from the system, and also 
// resolves links found in this information.
size_t ls_dir_get_files(struct ls_state* state, struct ls_dir* this_dir, int rlvl) {
    size_t count = 0;       // number of files in this directory
    size_t count_full = 0;  // number of files recursively in this directory
    size_t max = 0;         // max number of files in this directory
    struct ls_file* curfile;// operating file object
    char stat_path[PATH_MAX];//path to give to stat()
    char rl_buf[PATH_MAX];  // buffer to give to readlink()
    size_t name_len = 0;    // length of dir name
    DIR* dp;
    struct dirent *ep;
    // Reset block count
    this_dir->blocks = 0;
    if (state->options.stat_files == 1) {
        // Pre-copy the directory name into the stat path
        strcpy(stat_path,this_dir->name);
        name_len = strlen(stat_path);
    }
    dp = opendir(this_dir->name);
#if LS_BUILD_GIT
    if (state->options.git_open) {
        int giterr = git_repository_open_ext(&(this_dir->repo), this_dir->name, 0, NULL);
        if (giterr == -3) {
            this_dir->repo = NULL;
        } else if (giterr < 0) {
            ls_do_git_error(state, giterr);
        }
    }
#endif
    if (dp != NULL) {
        while ((ep = readdir(dp))) {
            // If we encounter an error finding a file, just mark it as invalid.
            int valid = 1;
            // If -a is specified, print all entries if they start with "."
            // If -A is specified, print all entries if they aren't ".." or "."
            // I could have used strcmp() here but this is faster.
            if (state->options.all_files == 0) {
                if (state->options.almost_all_files == 0) {
                    if (ep->d_name[0] == '.') {
                        continue;
                    }
                }
                if (ep->d_name[0] == '.') {
                    if (ep->d_name[1] == '\0') {
                        continue;
                    } else if (ep->d_name[1] == '.') {
                        if (ep->d_name[2] == '\0') {
                            continue;
                        }
                    }
                }
            }
            max++;
            if (rlvl > 0) {
                if (state->options.listing_tree_max_files != 0) {
                    if (count >= state->options.listing_tree_max_files) {
                        continue;
                    }
                }
            }
            curfile = malloc(sizeof(struct ls_file));
            curfile->valid = 1;
            curfile->link_to = NULL;
            curfile->child = NULL;
            // Copy name string to file struct name string.
            curfile->name = malloc(sizeof(char)*(strlen(ep->d_name)+1));
            if (curfile->name == NULL) {
                ls_do_error(state, "error allocating files memory");
            }
            strcpy(curfile->name,ep->d_name);
            // Stat the file if statting is needed.
            errno = 0;
            if (state->options.stat_files) {
                strcat(stat_path,curfile->name); 
                if (state->options.link_type == LS_LINK_RSTAT) {
                    stat(stat_path,&curfile->status);
                } else if (state->options.link_type == LS_LINK_LSTAT) {
                    lstat(stat_path,&curfile->status);
                } else {
                    lstat(stat_path,&curfile->status);
                }
                if (state->options.resolve_links) {
                    if (S_ISLNK(curfile->status.st_mode)) {
                        // Ugly code here basically resolves the link and stores
                        // it into the link_to buffer. This is used in printing
                        // (e.g. "-> link")
                        int rv = readlink(stat_path,&(rl_buf[0]),PATH_MAX);
                        if (rv == -1) {
                            ls_do_warning(state,"could not read link");
                            valid = 0;
                        } else {
                            int sl = strlen(&(rl_buf[0]));
                            curfile->link_to = malloc(sizeof(char)*(sl+1));
                            curfile->link_to[sl] = '\0';
                            if (curfile->link_to == NULL) {
                                ls_do_error(state,"could not allocate link memory");
                            }
                            strcpy(curfile->link_to,&(rl_buf[0]));
                            memset(rl_buf, 0, sizeof(rl_buf));
                        }
                    }
                }
                this_dir->blocks += curfile->status.st_blocks;
            }
            if (valid) {
                curfile->clschar[0] = '\0';
                // Get the file's extension.
                char *dot = strrchr(curfile->name, '.');
                if((dot == NULL) || (dot == curfile->name)) {
                    curfile->ext = curfile->name;
                } else {
                    curfile->ext = dot;
                }
#if LS_BUILD_GIT
                curfile->git_status = 0;
                if (this_dir->repo) {
                    int start = 0;
                    if (stat_path[0] == '.') {
                        if (stat_path[1] == '/') {
                            start = 2;
                        }
                    }
                    int giterr = git_status_file(&(curfile->git_status), this_dir->repo, &(stat_path[start]));
                    if (giterr < 0 && giterr != -3 && giterr != -5) {
                        curfile->git_status = 0;
                        ls_do_git_warning(state, giterr);
                    }
                }
#endif
                // Set the appropriate entry in the files_ptr table.
                sb_push(this_dir->files_ptr, curfile);
                count++;
                count_full++;
                if (state->options.stat_files && state->options.listing_tree) {
                    if (S_ISDIR(curfile->status.st_mode)) {
                        curfile->child = ls_dir_make(state, stat_path, 0, curfile->status.st_ino);
                        if (rlvl < state->options.listing_tree_max_depth) {
                            int out = ls_dir_get_files(state, curfile->child, rlvl+1);
                            if (out == 0) {
                                ls_dir_free_files(state, curfile->child);
                                ls_dir_free_rest(state, curfile->child);
                                curfile->child = NULL;
                            } else {
                                count_full += ls_dir_get_files(state, curfile->child, rlvl+1);
                            }
                        }
                    }
                }
            } else {
                free(curfile->name);
                free(curfile);
            }
            if (state->options.stat_files) {
                // Trick strcat into overwriting the last path the next 
                // time around this loop
                stat_path[name_len] = '\0';
            }
        }
        (void)closedir(dp);
    } else {
        int len = PATH_MAX+30;
        char msg[len];
        snprintf(&(msg[0]), len, "cannot access '%s'", this_dir->cmdline_name);
        ls_do_warning(state, &(msg[0]));
    }
#if LS_BUILD_GIT
    if (state->options.git_open) {
        if (this_dir->repo) {
            git_repository_free(this_dir->repo);
        }
    }
#endif
    // The only reason I'm not using sb_count here is because at this point it
    // is not certain that sb_push has been called yet. Thus, the "call" to
    // sb_count could fail on the pretense that it's not able to actually access
    // the "special" memory before the buffer that sb_push uses.
    this_dir->count = count;
    this_dir->count_full = count_full;
    this_dir->max = max;
    return count_full;
}

// Frees file information present in the directory listing.
void ls_dir_free_files(struct ls_state* state, struct ls_dir* dir_obj) {
    if (state) {
        if (dir_obj->count != 0) {
            for (unsigned int i=0;i<dir_obj->count;i++) {
                if (dir_obj->files_ptr[i]->child != NULL) {
                    ls_dir_free_files(state, dir_obj->files_ptr[i]->child);
                    ls_dir_free_rest(state, dir_obj->files_ptr[i]->child);
                }
                free(dir_obj->files_ptr[i]->name);
                free(dir_obj->files_ptr[i]->link_to);
                free(dir_obj->files_ptr[i]);
            }
            sb_free(dir_obj->files_ptr);
        }
    }
}

// Frees the rest of the directory information (child table object, name)
void ls_dir_free_rest(struct ls_state* state, struct ls_dir* this_dir) {
    if (state) {
        free(this_dir->name);
        for (size_t i=0;i<this_dir->tbl.count;i++) {
            free(this_dir->tbl.children_ptr[i]);
        }
        sb_free(this_dir->tbl.children_ptr);
    }
}

// The next functions are sort key functions to be passed into qsort(). They are
// controlled by the sorting command line options (which I can't remember off
// the top of my head). Nevertheless, they are not static because they are
// directly used by the program option generator.

// Default sort: Alphabetical, **BY REGION (strcoll())**
int ls_dir_sort_key_default(void* A, void* B) {
    struct ls_file* a = *((struct ls_file**)A);
    struct ls_file* b = *((struct ls_file**)B);
    return strcoll(
        a->name,
        b->name
    );
}

// Size sort
int ls_dir_sort_key_size(void* A, void* B) {
    struct ls_file* a = *((struct ls_file**)A);
    struct ls_file* b = *((struct ls_file**)B);
    int sg = SGN((a->status.st_size)-(b->status.st_size));
    if (sg == 0) {
        return strcoll(
            a->name,
            b->name
        );
    }
    return sg;
}

// Last modification time sort
int ls_dir_sort_key_mtime(void* A, void* B) {
    struct ls_file* a = *((struct ls_file**)A);
    struct ls_file* b = *((struct ls_file**)B);
    double sg = SGN(difftime(a->status.st_mtime,b->status.st_mtime));
    if (sg == 0) {
        return strcoll(
            a->name,
            b->name
        );
    }
    return sg;
}

// CTime (idk) sort
int ls_dir_sort_key_ctime(void* A, void* B) {
    struct ls_file* a = *((struct ls_file**)A);
    struct ls_file* b = *((struct ls_file**)B);
    double sg = SGN(difftime(a->status.st_ctime,b->status.st_ctime));
    if (sg == 0) {
        return strcoll(
            a->name,
            b->name
        );
    }
    return sg;
}

// Last access time sort
int ls_dir_sort_key_atime(void* A, void* B) {
    struct ls_file* a = *((struct ls_file**)A);
    struct ls_file* b = *((struct ls_file**)B);
    double sg = SGN(difftime(a->status.st_atime,b->status.st_atime));
    if (sg == 0) {
        return strcoll(
            a->name,
            b->name
        );
    }
    return sg;
}

// Actually sort the fucking files
void ls_dir_sort(struct ls_state* state, struct ls_dir* this_dir) {
    if (state->options.sort_key == NULL) {
        return;
    }
    if (this_dir->count < 2) {
        return;
    }
    qsort(
        this_dir->files_ptr,
        this_dir->count,
        sizeof(struct ls_file*),
        (int (*)(const void*, const void*))state->options.sort_key
    );
    if (state->options.sort_reverse) {
        int left = 0;
        int right = this_dir->count-1;
        while (left < right) {
            struct ls_file* temp = this_dir->files_ptr[left];
            this_dir->files_ptr[left++] = this_dir->files_ptr[right];
            this_dir->files_ptr[right--] = temp;
        }
    }
    if (state->options.listing_tree) {
        for (size_t i = 0; i < this_dir->count; i++) {
            if (this_dir->files_ptr[i]->child != NULL) {
                ls_dir_sort(state, this_dir->files_ptr[i]->child);
            }
        }
    }
}

// I guess this function prepares all the data needed for do_dir_table when 
// using the -R flag. Basically skips over the .. and . entries if using -a/-A, 
// and creates a new dir object for each directory found in the dir listing.
void ls_dir_prep_recur(struct ls_state* state, struct ls_dir* this_dir) {
    for (unsigned int i=0;i<this_dir->count;i++) {
        struct ls_file* curfile = this_dir->files_ptr[i];
        if (strcmp(curfile->name,".") == 0) {
            continue;
        }
        if (strcmp(curfile->name,"..") == 0) {
            continue;
        }
        if (S_ISDIR(curfile->status.st_mode)) {
            char* new_dir_name = malloc(strlen(curfile->name)+strlen(this_dir->name)+2);
            strcpy(new_dir_name,this_dir->name);
            strcat(new_dir_name,curfile->name);
            ls_dir_table_add(state,&(this_dir->tbl),new_dir_name,0,curfile->status.st_ino);
            free(new_dir_name); // new_dir_name is copied to a new buffer, we won't need it
        }
    }
}