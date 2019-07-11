#if LS_BUILD_GIT
#include "git2.h"
#endif

#include "ls_state.h"

int main(int argc, char** argv) {
    struct ls_state state;
    ls_state_init(&state,argv[0]);
    ls_options_prepare(&state);
    ls_options_get(&state,argc,argv);
    ls_dir_table_eval(&state,&(state.cmdline),0);
    ls_printer_flush(&state);
    if (state.options.listing_type == LS_LISTING_STREAM) {
        printf("\n");
    }
#if LS_BUILD_GIT
    if (state.options.git_init) {
        git_libgit2_shutdown();
    }
#endif
    return 0;
}
