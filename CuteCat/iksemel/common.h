/* iksemel (XML parser for Jabber)
** Copyright (C) 2000-2003 Gurer Ozen <madcat@e-kolay.net>
** This code is free software; you can redistribute it and/or
** modify it under the terms of GNU Lesser General Public License.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <stdio.h>

#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#elif HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifndef errno
extern int errno;
#endif

#include "finetune.h"

/*add by will*/
#ifdef WIN32
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#define strncasecmp strnicmp
#define strcasecmp stricmp
#endif
/*end add*/