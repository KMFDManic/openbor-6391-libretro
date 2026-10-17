/*
 * OpenBOR - http://www.LavaLit.com
 * -----------------------------------------------------------------------
 * All rights reserved, see LICENSE in OpenBOR root for details.
 *
 * Copyright (c) 2004 - 2011 OpenBOR Team
 */

/////////////////////////////////////////////////////////////////////////////

#ifndef GLOBALS_H
#define GLOBALS_H

/////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <ctype.h>

#include "utils.h"

#ifdef PSP
#include <stdarg.h>
#include <psppower.h>
#include "pspport.h"
#include "graphics.h"
#endif

#ifdef SDL
#include "sdlport.h"
#endif

#ifdef GP2X
#include "gp2xport.h"
#endif

#ifdef DOS
#include "dosport.h"
#endif

#ifdef DC
#include "dcport.h"
#endif

#ifdef XBOX
#include "xboxport.h"
#endif

#ifdef LIBRETRO
#include "libretroport.h"
#endif

#ifdef WII
#include <gctypes.h>
#include <ogc/conf.h>
#include "wiiport.h"
#endif

/////////////////////////////////////////////////////////////////////////////

#define printf writeToLogFile

// Path buffers that hold absolute save paths. The libretro save directory
// can be long, so they are enlarged there.
#ifdef LIBRETRO
#define OB_PATHBUF128 1024
#define OB_PATHBUF256 1024
#else
#define OB_PATHBUF128 128
#define OB_PATHBUF256 256
#endif

/////////////////////////////////////////////////////////////////////////////

extern int int_assert[sizeof(int)==4?1:-1];

#define MIN_INT (int)0x80000000
#define MAX_INT	(int)0x7fffffff

#endif
