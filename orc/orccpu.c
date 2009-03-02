/*
 * ORC - Library of Optimized Inner Loops
 * Copyright (c) 2003,2004 David A. Schleef <ds@schleef.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <orc/orcdebug.h>
#include <orc/orccpu.h>
#include <orc/orcutils.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <time.h>

#if defined(__FreeBSD__)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#ifdef __sun
#include <sys/auxv.h>
#endif



/**
 * SECTION:liborccpu
 * @title: CPU
 * @short_description: Check the capabilities of the current CPU
 *
 */

void orc_cpu_detect_arch(void);

unsigned long orc_cpu_flags;

#if 0
extern unsigned long (*_orc_profile_stamp)(void);
#endif

#if 0
#ifdef HAVE_GETTIMEOFDAY
static unsigned long
orc_profile_stamp_gtod (void)
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return 1000000*(unsigned long)tv.tv_sec + (unsigned long)tv.tv_usec;
}
#endif

#if defined(HAVE_CLOCK_GETTIME) && defined(HAVE_MONOTONIC_CLOCK)
static unsigned long
orc_profile_stamp_clock_gettime (void)
{
  struct timespec ts;
  clock_gettime (CLOCK_MONOTONIC, &ts);
  return 1000000000*ts.tv_sec + ts.tv_nsec;
}
#endif

static unsigned long
orc_profile_stamp_zero (void)
{
  return 0;
}
#endif

void
_orc_cpu_init (void)
{
  const char *envvar;

  orc_cpu_detect_arch();

  envvar = getenv ("ORC_CPU_FLAGS");
  if (envvar != NULL) {
    char *end = NULL;
    unsigned long flags;

    flags = strtoul (envvar, &end, 0);
    if (end > envvar) {
      orc_cpu_flags = flags;
    }
    ORC_INFO ("cpu flags from environment %08lx", orc_cpu_flags);
  }

  ORC_INFO ("cpu flags %08lx", orc_cpu_flags);

#if 0
#if defined(HAVE_CLOCK_GETTIME) && defined(HAVE_MONOTONIC_CLOCK)
  if (_orc_profile_stamp == NULL) {
    _orc_profile_stamp = orc_profile_stamp_clock_gettime;
    ORC_INFO("Using clock_gettime() as a timestamp function.");
  }
#endif

#ifdef HAVE_GETTIMEOFDAY
  if (_orc_profile_stamp == NULL) {
    _orc_profile_stamp = orc_profile_stamp_gtod;
    ORC_WARNING("Using gettimeofday() as a timestamp function.");
  }
#endif
  if (_orc_profile_stamp == NULL) {
    _orc_profile_stamp = orc_profile_stamp_zero;
    ORC_ERROR("No timestamping function.  This is kinda bad.");
  }
#endif
}

/**
 * orc_cpu_get_flags:
 *
 * Returns a bitmask containing the available CPU features.
 *
 * Returns: the CPU features.
 */
unsigned int
orc_cpu_get_flags (void)
{
  return orc_cpu_flags;
}


