#include "ls_uid_gid.h"

#include "ls_state.h"

struct ls_uid_cache_page* ls_uidtbl_get(struct ls_state* state, uid_t id_in) {
    struct ls_uid_cache_page* target;
    HASH_FIND_INT(state->ucch, (int*)&id_in, target);
    if (target == NULL) {
        target = malloc(sizeof(struct ls_uid_cache_page));
        target->id = id_in;
        if (!state->options.print_ugid_numbers) {
            errno = 0;
            struct passwd* temp_pwd = getpwuid(id_in);
            if (temp_pwd == NULL) {
                ls_do_error(state, "Couldn't obtain UID");
            }
            size_t nlen = strlen(temp_pwd->pw_name);
            char* final_name = malloc(sizeof(char)*(nlen+1));
            if (final_name == NULL) {
                ls_do_error(state, "Couldn't allocate UID name");
            }
            strcpy(final_name, temp_pwd->pw_name);
            target->name = final_name;
            target->size = nlen;
        } else {
            char* final_name = malloc(sizeof(char)*LS_NSTR_MAX);
            if (final_name == NULL) {
                ls_do_error(state, "Couldn't allocate numeric UID name");
            }
            size_t nlen = snprintf(
                final_name, 
                LS_NSTR_MAX,
                "%u",
                id_in
            );
            target->name = final_name;
            target->size = nlen;
        }
        HASH_ADD_INT(state->ucch,id,target);
    }
    return target;
}

struct ls_gid_cache_page* ls_gidtbl_get(struct ls_state* state, gid_t id_in) {
    struct ls_gid_cache_page* target;
    HASH_FIND_INT(state->gcch, (int*)&id_in, target);
    if (target == NULL) {
        target = malloc(sizeof(struct ls_gid_cache_page));
        target->id = id_in;
        if (!state->options.print_ugid_numbers) {
            errno = 0;
            struct group* temp_grp = getgrgid(id_in);
            if (temp_grp == NULL) {
                ls_do_error(state, "Couldn't obtain GID");
            }
            size_t nlen = strlen(temp_grp->gr_name);
            char* final_name = malloc(sizeof(char)*(nlen+1));
            if (final_name == NULL) {
                ls_do_error(state, "Couldn't allocate GID name");
            }
            strcpy(final_name, temp_grp->gr_name);
            target->name = final_name;
            target->size = nlen;
        } else {
            char* final_name = malloc(sizeof(char)*LS_NSTR_MAX);
            if (final_name == NULL) {
                ls_do_error(state, "Couldn't allocate numeric GID name");
            }
            size_t nlen = snprintf(
                final_name, 
                LS_NSTR_MAX,
                "%u",
                id_in
            );
            target->name = final_name;
            target->size = nlen;
        }
        HASH_ADD_INT(state->gcch,id,target);
    }
    return target;
}