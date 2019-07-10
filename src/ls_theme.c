#include "ls_theme.h"
#include "ls_theme_parse.h"

#include "ls_common.h"
#include "ls_color.h"
#include "ls_state.h"

#define LS_THEMEVAL_MAX 256

char* ls_format_invalid = "\0";
char* ls_format_def_end = "0";
char* ls_format_def_left = "\x1b[";
char* ls_format_def_right = "m";

char* ls_extension_invalid = "\0";
char* ls_icon_invalid = "";

const char* ls_theme_verbose_names[LS_FORMAT_LAST] = {
    "mode_type_off",
    "mode_type_link",
    "mode_type_dir",
    "mode_type_chr",
    "mode_type_blk",
    "mode_type_sock",
    "mode_type_fifo",
    "mode_user_r_off",
    "mode_user_r_on",
    "mode_user_w_off",
    "mode_user_w_on",
    "mode_user_x_off",
    "mode_user_x_off_setuid",
    "mode_user_x_on",
    "mode_user_x_on_setuid",
    "mode_group_r_off",
    "mode_group_r_on",
    "mode_group_w_off",
    "mode_group_w_on",
    "mode_group_x_off",
    "mode_group_x_off_setgid",
    "mode_group_x_on",
    "mode_group_x_on_setgid",
    "mode_other_r_off",
    "mode_other_r_on",
    "mode_other_w_off",
    "mode_other_w_on",
    "mode_other_x_off",
    "mode_other_x_off_sticky",
    "mode_other_x_on",
    "mode_other_x_on_sticky",
    "file_normal",
    "file",
    "directory",
    "link",
    "fifo",
    "socket",
    "block_dev",
    "char_dev",
    "missing",
    "orphan",
    "executable",
    "door",
    "setuid",
    "setgid",
    "sticky",
    "other_writable",
    "sticky_other_writable",
    "capability",
    "multi_hard_link",
    "clrtoeol",
    "class_directory",
    "class_link",
    "class_fifo",
    "class_executable",
    "class_block_dev",
    "class_char_dev",
    "class_door",
    "header_path",
    "header_total",
    "inode",
    "blocks",
    "nlinks",
    "size",
    "size_unit",
    "owner",
    "group",
    "date",
    "link_arrow",
    "link_dest",
    "tree_more_paren",
    "tree_more_count",
    "tree_branch",
#if LS_BUILD_GIT
    "gs_index_none",
    "gs_index_unknown",
    "gs_index_new",
    "gs_index_modified",
    "gs_index_deleted",
    "gs_index_renamed",
    "gs_index_typechange",
    "gs_index_ignored",
    "gs_wt_none",
    "gs_wt_unknown",
    "gs_wt_new",
    "gs_wt_modified",
    "gs_wt_deleted",
    "gs_wt_renamed",
    "gs_wt_typechange",
    "gs_wt_ignored",
#endif
    "code_left",
    "code_right",
    "code_end",
    "reset",
};

int ls_theme_has_format(struct ls_state* state, int fmt) {
    return (state->theme.formats[fmt].flags & LS_FORMAT_FLAG_VALID);
}

void ls_theme_zero_format(struct ls_format* fmt, char* name) {
    fmt->name = name;
    fmt->icon = ls_icon_invalid;
    fmt->flags = LS_FORMAT_FLAG_VALID;
    fmt->border_left = NULL;
    fmt->border_right = NULL;
    fmt->fg.type = LS_COLOR_TYPE_SOLID;
    fmt->fg.colors = NULL;
    ls_color_iter_add(&(fmt->fg), ls_color_none());
    fmt->bg.type = LS_COLOR_TYPE_SOLID;
    fmt->bg.colors = NULL;
    ls_color_iter_add(&(fmt->bg), ls_color_none());
    fmt->ifg.type = LS_COLOR_TYPE_SOLID;
    fmt->ifg.colors = NULL;
    ls_color_iter_add(&(fmt->ifg), ls_color_none());
    fmt->ibg.type = LS_COLOR_TYPE_SOLID;
    fmt->ibg.colors = NULL;
    ls_color_iter_add(&(fmt->ibg), ls_color_none());
    fmt->efg.type = LS_COLOR_TYPE_SOLID;
    fmt->efg.colors = NULL;
    ls_color_iter_add(&(fmt->efg), ls_color_none());
    fmt->ebg.type = LS_COLOR_TYPE_SOLID;
    fmt->ebg.colors = NULL;
    ls_color_iter_add(&(fmt->ebg), ls_color_none());
}

static void ls_theme_zero_formats(struct ls_state* state) {
    for (int i = 0; i < LS_FORMAT_LAST; i++) {
        ls_theme_zero_format(&(state->theme.formats[i]),(char*)ls_theme_verbose_names[i]);
    }    
}

static void ls_theme_load_default(struct ls_state* state) {
    ls_theme_zero_formats(state);
    state->theme.formats[LS_FORMAT_RESET].flags |= LS_FORMAT_FLAG_RESET;
    struct ls_format* formats = state->theme.formats;
    ls_format_fgsol(&formats[LS_FORMAT_MODE_TYPE_OFF], ls_color_lofi(8));
    ls_format_fgsol(&formats[LS_FORMAT_MODE_TYPE_LINK], ls_color_lofi(6));
    ls_format_flag (&formats[LS_FORMAT_MODE_TYPE_LINK], LS_FORMAT_FLAG_BOLD);
    ls_format_fgsol(&formats[LS_FORMAT_MODE_TYPE_DIR], ls_color_lofi(4));
    ls_format_flag (&formats[LS_FORMAT_MODE_TYPE_DIR], LS_FORMAT_FLAG_BOLD);
    ls_format_fgsol(&formats[LS_FORMAT_MODE_TYPE_CHR], ls_color_lofi(3));
    ls_format_flag (&formats[LS_FORMAT_MODE_TYPE_CHR], LS_FORMAT_FLAG_BOLD);
    ls_format_fgsol(&formats[LS_FORMAT_MODE_TYPE_SOCK], ls_color_lofi(5));
    ls_format_flag (&formats[LS_FORMAT_MODE_TYPE_SOCK], LS_FORMAT_FLAG_BOLD);
    ls_format_fgsol(&formats[LS_FORMAT_MODE_TYPE_FIFO], ls_color_lofi(3));
    ls_format_fgsol(&formats[LS_FORMAT_MODE_USER_R_OFF], ls_color_lofi(8));
    ls_format_fgsol(&formats[LS_FORMAT_MODE_USER_W_OFF], ls_color_lofi(8));
    ls_format_fgsol(&formats[LS_FORMAT_MODE_USER_X_OFF], ls_color_lofi(8));
    ls_format_fgsol(&formats[LS_FORMAT_MODE_GROUP_R_OFF], ls_color_lofi(8));
    ls_format_fgsol(&formats[LS_FORMAT_MODE_GROUP_W_OFF], ls_color_lofi(8));
    ls_format_fgsol(&formats[LS_FORMAT_MODE_GROUP_X_OFF], ls_color_lofi(8));
    ls_format_fgsol(&formats[LS_FORMAT_MODE_OTHER_R_OFF], ls_color_lofi(8));
    ls_format_fgsol(&formats[LS_FORMAT_MODE_OTHER_W_OFF], ls_color_lofi(8));
    ls_format_fgsol(&formats[LS_FORMAT_MODE_OTHER_X_OFF], ls_color_lofi(8));
    ls_format_fgsol(&formats[LS_FORMAT_OBJ_DIR], ls_color_lofi(4));
    ls_format_flag (&formats[LS_FORMAT_OBJ_DIR], LS_FORMAT_FLAG_BOLD);
    ls_format_fgsol(&formats[LS_FORMAT_OBJ_LINK], ls_color_lofi(6));
    ls_format_flag (&formats[LS_FORMAT_OBJ_LINK], LS_FORMAT_FLAG_BOLD);
    ls_format_fgsol(&formats[LS_FORMAT_OBJ_FIFO], ls_color_lofi(3));
    ls_format_fgsol(&formats[LS_FORMAT_OBJ_SOCK], ls_color_lofi(5));
    ls_format_flag (&formats[LS_FORMAT_OBJ_SOCK], LS_FORMAT_FLAG_BOLD);
    ls_format_fgsol(&formats[LS_FORMAT_OBJ_BLK], ls_color_lofi(3));
    ls_format_flag (&formats[LS_FORMAT_OBJ_BLK], LS_FORMAT_FLAG_BOLD);
    ls_format_flag (&formats[LS_FORMAT_OBJ_BLK], LS_FORMAT_FLAG_UNDERLINE);
    ls_format_fgsol(&formats[LS_FORMAT_OBJ_CHR], ls_color_lofi(3));
    ls_format_flag (&formats[LS_FORMAT_OBJ_CHR], LS_FORMAT_FLAG_BOLD);
    ls_format_fgsol(&formats[LS_FORMAT_OBJ_EXEC], ls_color_lofi(2));
    ls_format_flag (&formats[LS_FORMAT_OBJ_EXEC], LS_FORMAT_FLAG_BOLD);
    ls_format_fgsol(&formats[LS_FORMAT_OBJ_DOOR], ls_color_lofi(5));
    ls_format_flag (&formats[LS_FORMAT_OBJ_DOOR], LS_FORMAT_FLAG_BOLD);
    ls_format_fgsol(&formats[LS_FORMAT_OBJ_SETUID], ls_color_lofi(7));
    ls_format_bgsol(&formats[LS_FORMAT_OBJ_SETUID], ls_color_lofi(1));
    ls_format_fgsol(&formats[LS_FORMAT_OBJ_SETGID], ls_color_lofi(0));
    ls_format_bgsol(&formats[LS_FORMAT_OBJ_SETGID], ls_color_lofi(3));
    ls_format_fgsol(&formats[LS_FORMAT_OBJ_STICKY], ls_color_lofi(7));
    ls_format_bgsol(&formats[LS_FORMAT_OBJ_STICKY], ls_color_lofi(4));
    ls_format_fgsol(&formats[LS_FORMAT_OBJ_OTHER_WRITE], ls_color_lofi(4));
    ls_format_bgsol(&formats[LS_FORMAT_OBJ_OTHER_WRITE], ls_color_lofi(2));
    ls_format_fgsol(&formats[LS_FORMAT_OBJ_STICKY_OTHER_WRITE], ls_color_lofi(0));
    ls_format_bgsol(&formats[LS_FORMAT_OBJ_STICKY_OTHER_WRITE], ls_color_lofi(2));
    ls_format_fgsol(&formats[LS_FORMAT_LINK_ARROW], ls_color_lofi(8));
    ls_format_fgsol(&formats[LS_FORMAT_LINK_DEST], ls_color_lofi(6));
    ls_format_flag (&formats[LS_FORMAT_LINK_DEST], LS_FORMAT_FLAG_BOLD);
    ls_format_fgsol(&formats[LS_FORMAT_TREE_MORE_PAREN], ls_color_lofi(7));
    ls_format_flag (&formats[LS_FORMAT_TREE_MORE_PAREN], LS_FORMAT_FLAG_ITALIC);
    ls_format_fgsol(&formats[LS_FORMAT_TREE_MORE_COUNT], ls_color_lofi(13));
    ls_format_flag (&formats[LS_FORMAT_TREE_MORE_COUNT], LS_FORMAT_FLAG_ITALIC);
    ls_format_fgsol(&formats[LS_FORMAT_TREE_BRANCH], ls_color_lofi(7));
    ls_format_fgsol(&formats[LS_FORMAT_SIZE_UNIT], ls_color_lofi(7));
    ls_format_fgsol(&formats[LS_FORMAT_CLASS_DIR], ls_color_lofi(4));
    ls_format_flag (&formats[LS_FORMAT_CLASS_DIR], LS_FORMAT_FLAG_BOLD);
    ls_format_fgsol(&formats[LS_FORMAT_CLASS_LINK], ls_color_lofi(6));
    ls_format_flag (&formats[LS_FORMAT_CLASS_LINK], LS_FORMAT_FLAG_BOLD);
    ls_format_fgsol(&formats[LS_FORMAT_CLASS_FIFO], ls_color_lofi(3));
    ls_format_flag (&formats[LS_FORMAT_CLASS_FIFO], LS_FORMAT_FLAG_BOLD);
    ls_format_fgsol(&formats[LS_FORMAT_CLASS_SOCK], ls_color_lofi(5));
    ls_format_flag (&formats[LS_FORMAT_CLASS_SOCK], LS_FORMAT_FLAG_BOLD);
    ls_format_fgsol(&formats[LS_FORMAT_CLASS_EXEC], ls_color_lofi(2));
    ls_format_flag (&formats[LS_FORMAT_CLASS_EXEC], LS_FORMAT_FLAG_BOLD);
    ls_format_fgsol(&formats[LS_FORMAT_CLASS_SOCK], ls_color_lofi(5));
    ls_format_flag (&formats[LS_FORMAT_CLASS_SOCK], LS_FORMAT_FLAG_BOLD);
    ls_format_fgsol(&formats[LS_FORMAT_CLASS_BLK], ls_color_lofi(3));
    ls_format_flag (&formats[LS_FORMAT_CLASS_BLK], LS_FORMAT_FLAG_BOLD);
    ls_format_flag (&formats[LS_FORMAT_CLASS_BLK], LS_FORMAT_FLAG_UNDERLINE);
    ls_format_fgsol(&formats[LS_FORMAT_CLASS_CHR], ls_color_lofi(3));
    ls_format_flag (&formats[LS_FORMAT_CLASS_CHR], LS_FORMAT_FLAG_BOLD);
}

void ls_theme_load(struct ls_state* state) {
    // Allocate formats
    state->theme.formats = malloc(sizeof(struct ls_format)*LS_FORMAT_LAST);
    state->theme.extensions = NULL;
    errno = 0;
    if (state->theme.formats == NULL) {
        ls_do_error(state,"couldn't allocate formats list");
    }
    if (state->options.config_file != NULL) {
        ls_theme_zero_formats(state);
        FILE* fp = fopen(state->options.config_file,"r");
        if (errno) {
            ls_do_warning(state,"could not open ls_colors.json");
            fclose(fp);
        } else {
            ls_theme_parse_file(state, fp);
        }
    } else {
        ls_theme_load_default(state);
    }
}