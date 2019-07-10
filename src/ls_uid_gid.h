#ifndef LS_UID_GID_H
#define LS_UID_GID_H

#include "uthash.h"

#include "ls_common.h"

struct ls_state;

struct ls_uid_cache_page {
    int id;                         // UID of this page (cast from uid_t)
    size_t size;                    // Size of the text
    char* name;                     // User name
    UT_hash_handle hh;              // UTHash handle
};

struct ls_gid_cache_page {
    int id;                         // GID of this page (cast from gid_t)
    size_t size;                    // Size of the text
    char* name;                     // Group name
    UT_hash_handle hh;              // UTHash handle
};

struct ls_uid_cache_page* ls_uidtbl_get(struct ls_state* state, uid_t id);

struct ls_gid_cache_page* ls_gidtbl_get(struct ls_state* state, gid_t id);

#endif