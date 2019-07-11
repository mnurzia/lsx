#ifndef LS_COMMON_H
#define LS_COMMON_H

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <grp.h>
#include <limits.h>
#include <math.h>
#include <pwd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include "ls_config.h"

// Completion:
// -A -- DONE
// -C -- DONE
// -F -- DONE
// -H -- DONE
// -L -- DONE
// -R -- DONE
// -S -- DONE
// -U -- DONE
// -a -- DONE
// -c -- DONE
// -d --        Need help with this one... not sure about what it does
// -f -- DONE
// -g -- DONE
// -h -- 
// -i -- DONE
// -k -- 
// -l -- DONE
// -m -- DONE
// -n -- DONE
// -o -- DONE
// -p -- DONE
// -q -- 
// -r -- DONE
// -s -- DONE
// -t -- DONE
// -u -- DONE
// -x -- DONE
// -1 -- DONE
// --color -- DONE
// --icons -- DONE
// --help -- DONE

// Helpful macros:
#define MAX(x,y) (((x)>(y))?(x):(y))
#define MIN(x,y) (((x)<(y))?(x):(y))
#define CEIL_DIVIDE(x,y) (((x) + (y) - 1) / (y))
#define SGN(x) (((x) > 0) - ((x) < 0))
#define LERP(x,y,t) ((float)(x)+((float)(y)-(float)(x))*((float)t))

#endif