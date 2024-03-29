#ifndef LS_THEME_H
#define LS_THEME_H

#include "ls_common.h"

#include "ls_format.h"

struct ls_state;

enum {
    LS_FORMAT_MODE_TYPE_OFF,
    LS_FORMAT_MODE_TYPE_LINK,
    LS_FORMAT_MODE_TYPE_DIR,
    LS_FORMAT_MODE_TYPE_CHR,
    LS_FORMAT_MODE_TYPE_BLK,
    LS_FORMAT_MODE_TYPE_SOCK,
    LS_FORMAT_MODE_TYPE_FIFO,
    LS_FORMAT_MODE_USER_R_OFF,
    LS_FORMAT_MODE_USER_R_ON,
    LS_FORMAT_MODE_USER_W_OFF,
    LS_FORMAT_MODE_USER_W_ON,
    LS_FORMAT_MODE_USER_X_OFF,
    LS_FORMAT_MODE_USER_X_OFF_SETUID,
    LS_FORMAT_MODE_USER_X_ON,
    LS_FORMAT_MODE_USER_X_ON_SETUID,
    LS_FORMAT_MODE_GROUP_R_OFF,
    LS_FORMAT_MODE_GROUP_R_ON,
    LS_FORMAT_MODE_GROUP_W_OFF,
    LS_FORMAT_MODE_GROUP_W_ON,
    LS_FORMAT_MODE_GROUP_X_OFF,
    LS_FORMAT_MODE_GROUP_X_OFF_SETGID,
    LS_FORMAT_MODE_GROUP_X_ON,
    LS_FORMAT_MODE_GROUP_X_ON_SETGID,
    LS_FORMAT_MODE_OTHER_R_OFF,
    LS_FORMAT_MODE_OTHER_R_ON,
    LS_FORMAT_MODE_OTHER_W_OFF,
    LS_FORMAT_MODE_OTHER_W_ON,
    LS_FORMAT_MODE_OTHER_X_OFF,
    LS_FORMAT_MODE_OTHER_X_OFF_STICKY,
    LS_FORMAT_MODE_OTHER_X_ON,
    LS_FORMAT_MODE_OTHER_X_ON_STICKY,
    LS_FORMAT_OBJ_NORMAL,
    LS_FORMAT_OBJ_FILE,
    LS_FORMAT_OBJ_DIR,
    LS_FORMAT_OBJ_LINK,
    LS_FORMAT_OBJ_FIFO,
    LS_FORMAT_OBJ_SOCK,
    LS_FORMAT_OBJ_BLK,
    LS_FORMAT_OBJ_CHR,
    LS_FORMAT_OBJ_MISSING,
    LS_FORMAT_OBJ_ORPHAN,
    LS_FORMAT_OBJ_EXEC,
    LS_FORMAT_OBJ_DOOR,
    LS_FORMAT_OBJ_SETUID,
    LS_FORMAT_OBJ_SETGID,
    LS_FORMAT_OBJ_STICKY,
    LS_FORMAT_OBJ_OTHER_WRITE,
    LS_FORMAT_OBJ_STICKY_OTHER_WRITE,
    LS_FORMAT_OBJ_MULTI_HARD_LINK,
    LS_FORMAT_OBJ_CAPABILITY,
    LS_FORMAT_CLEAR_TO_EOL,
    LS_FORMAT_CLASS_DIR,
    LS_FORMAT_CLASS_LINK,
    LS_FORMAT_CLASS_FIFO,
    LS_FORMAT_CLASS_SOCK,
    LS_FORMAT_CLASS_EXEC,
    LS_FORMAT_CLASS_BLK,
    LS_FORMAT_CLASS_CHR,
    LS_FORMAT_CLASS_DOOR,
    LS_FORMAT_HEADER_PATH,
    LS_FORMAT_HEADER_TOTAL,
    LS_FORMAT_INODE,
    LS_FORMAT_BLOCKS,
    LS_FORMAT_NLINKS,
    LS_FORMAT_SIZE,
    LS_FORMAT_SIZE_UNIT,
    LS_FORMAT_OWNER,
    LS_FORMAT_GROUP,
    LS_FORMAT_DATE,
    LS_FORMAT_LINK_ARROW,
    LS_FORMAT_LINK_DEST,
    LS_FORMAT_TREE_MORE_PAREN,
    LS_FORMAT_TREE_MORE_COUNT,
    LS_FORMAT_TREE_BRANCH,
#if LS_BUILD_GIT
    LS_FORMAT_GIT_STATUS_INDEX_NONE,
    LS_FORMAT_GIT_STATUS_INDEX_UNKNOWN,
    LS_FORMAT_GIT_STATUS_INDEX_NEW,
    LS_FORMAT_GIT_STATUS_INDEX_MODIFIED,
    LS_FORMAT_GIT_STATUS_INDEX_DELETED,
    LS_FORMAT_GIT_STATUS_INDEX_RENAMED,
    LS_FORMAT_GIT_STATUS_INDEX_TYPECHANGE,
    LS_FORMAT_GIT_STATUS_INDEX_IGNORED,
    LS_FORMAT_GIT_STATUS_WT_NONE,
    LS_FORMAT_GIT_STATUS_WT_UNKNOWN,
    LS_FORMAT_GIT_STATUS_WT_NEW,
    LS_FORMAT_GIT_STATUS_WT_MODIFIED,
    LS_FORMAT_GIT_STATUS_WT_DELETED,
    LS_FORMAT_GIT_STATUS_WT_RENAMED,
    LS_FORMAT_GIT_STATUS_WT_TYPECHANGE,
    LS_FORMAT_GIT_STATUS_WT_IGNORED,
#endif
    LS_FORMAT_LEFT,
    LS_FORMAT_RIGHT,
    LS_FORMAT_END,
    LS_FORMAT_RESET,
    LS_FORMAT_LAST
};

extern char* ls_format_invalid;
extern char* ls_format_def_end;
extern char* ls_format_def_left;
extern char* ls_format_def_right;

extern char* ls_extension_invalid;
extern char* ls_icon_invalid;

extern const char* ls_theme_verbose_names[LS_FORMAT_LAST];

struct ls_theme {
    struct ls_format* extensions;
    struct ls_format* formats;
    int extension_count;
    int extension_allocsize;
};

int ls_theme_has_format(struct ls_state* state, int fmt);

void ls_theme_zero_format(struct ls_format* fmt, char* name);

void ls_theme_load(struct ls_state* state);

#endif