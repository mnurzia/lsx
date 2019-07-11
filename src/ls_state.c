#include "ls_state.h"

// Initialize the state object. Typically just resets everything to a vanilla
// LS mode. Also sets up "glue logic" like the command line arguments, program
// name, and base path.
void ls_state_init(struct ls_state* state, char* progname) {
    state->ucch = NULL;
    state->gcch = NULL;
    
    state->options.all_files            = 0;
    state->options.almost_all_files     = 0;
    state->options.classify_type        = LS_CLASSIFY_NONE;
    state->options.print_inodes         = 0;
    state->options.listing_type         = LS_LISTING_REGULAR;
    state->options.preprocess_data      = 0;
    state->options.print_blocks         = 0;
    state->options.print_headers        = 0;
    state->options.link_type            = LS_LINK_LSTAT;
    state->options.sort_key             = ls_dir_sort_key_default;
    state->options.stat_files           = 0;
    state->options.date_type            = LS_DATE_MTIME;
    state->options.print_ugid_numbers   = 0;
    state->options.sort_reverse         = 0;
    state->options.print_force_oneline  = 0;
    state->options.ncols                = 80;
    state->options.print_owner          = 1;
    state->options.print_group          = 1;
    state->options.recursive            = 0;
    state->options.format_type          = 0;
    state->options.print_icons          = 0;
    state->options.resolve_links        = 0;
    state->options.print_block_numbers  = 0;
    state->options.size_type            = LS_SIZE_REGULAR;
    state->options.size_power           = 1024;
    state->options.size_units           = ls_units_regular;
    state->options.color_mode           = 0;
    state->options.listing_tree         = 0;
    state->options.listing_tree_max_depth = 1;
    state->options.listing_tree_max_files = 5;
#if LS_BUILD_GIT
    state->options.git_open = 0;
    state->options.git_init = 0;
#endif

    state->cmdline.count = 0;
    state->cmdline.children_ptr = NULL;
    
    state->progname = progname;
    
    state->pp_state.initialized = 0;
    
    state->pr_state.size = 0;
    for (int i = 0; i < 128; i++) {
        state->pr_state.tree_state[i] = LS_PRINTER_BRANCH_NULL;
    }
#define LS_PATH_MAX_DIRS PATH_MAX/2
    state->path = malloc(sizeof(ino_t)*LS_PATH_MAX_DIRS);
    if (state->path == NULL) {
        ls_do_error(state,"couldn't allocate path memory");
    }
    
    state->theme.extension_count = 0;
    
    struct winsize sz;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &sz);
    state->options.ncols = sz.ws_col-2;
    
    state->options.config_file = NULL;
}