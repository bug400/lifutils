#ifndef _CONFIG_H
#define _CONFIG_H 1

#cmakedefine HAVE_UNISTD_H 1
#cmakedefine HAVE_GETOPT_F 1
#cmakedefine HAVE_GETLINE_F 1
#cmakedefine HAVE_IO_H 1
#cmakedefine HAVE_SETMODE 1
#cmakedefine HAVE__SETMODE 1
#cmakedefine HAVE_FILENO 1
#cmakedefine HAVE__FILENO 1
#cmakedefine HAVE_BASETSD_H 1
#cmakedefine HAVE__STRICMP_F 1
#cmakedefine HAVE__STRNICMP_F 1
#cmakedefine HAVE__MAX_PATH 1

#ifndef HAVE__SETMODE
#ifdef HAVE_SETMODE
#define _setmode setmode
#define HAVE__SETMODE 1
#endif /* HAVE_SETMODE */
#endif /* HAVE__SETMODE */

#ifndef HAVE__FILENO
#ifdef HAVE_FILENO
#define _fileno fileno
#define HAVE__FILENO 1
#endif /* HAVE_FILENO */
#endif /* HAVE__FILENO */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

#ifdef HAVE_BASETSD_H
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif /* HAVE_BASETSD_H */

#ifdef HAVE__SETMODE
#ifdef HAVE_IO_H
#include <io.h>
#endif /* HAVE_IO_H */
#define SETMODE_STDIN_BINARY _setmode(_fileno(stdin), O_BINARY)
#define SETMODE_STDOUT_BINARY _setmode(_fileno(stdout), O_BINARY)
#else 
#define SETMODE_STDIN_BINARY ;
#define SETMODE_STDOUT_BINARY ;
#endif /* HAVE__SETMODE */

#ifndef S_IRUSR
#define S_IRUSR 0
#endif

#ifndef S_IRGRP
#define S_IRGRP 0
#endif

#ifndef S_IWUSR
#define S_IWUSR 0
#endif

#ifndef S_IROTH
#define S_IROTH 0
#endif

#ifndef HAVE_GETOPT_F
#include "getopt.h"
#endif

#ifndef HAVE_GETLINE_F
#include "getline.h"
#endif

#ifdef HAVE__STRICMP_F
#define strcasecmp _stricmp
#endif

#ifdef HAVE__STRNICMP_F
#define strncasecmp _strnicmp
#endif

#ifdef HAVE__MAX_PATH
#define PATH_MAX _MAX_PATH
#endif

#endif /* _CONFIG_H */
