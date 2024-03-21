/*
 * ORC - Oil Runtime Compiler
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
#include <orc/orcarm.h>
#include <orc/orcutils.h>
#include <orc/orcdebug.h>
#include <orc/orcutils-private.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <time.h>
#if defined(__linux__)
#include <linux/auxvec.h>
#endif
#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

/***** arm *****/

#if defined (__arm__) || defined (__aarch64__) || defined (_M_ARM64)
#if 0
static unsigned long
orc_profile_stamp_xscale(void)
{
  unsigned int ts;
  __asm__ __volatile__ (
      "  mrc p14, 0, %0, c1, c0, 0 \n"
      : "=r" (ts));
  return ts;
}
#endif

#if defined(__linux__)
static unsigned long
orc_check_neon_proc_auxv (void)
{
  unsigned long flags = 0;
  unsigned long aux[2];
  ssize_t count;
  int fd;

  fd = open("/proc/self/auxv", O_RDONLY);
  if (fd < 0) {
    ORC_LOG ("Failed to open /proc/self/auxv");
    return 0;
  }

  while (1) {
    count = read(fd, aux, sizeof(aux));
    if (count < sizeof(aux)) {
      break;
    }

    if (aux[0] == AT_HWCAP) {
#ifdef __arm__
      /* if (aux[1] & 64) flags |= ORC_TARGET_NEON_VFP; */
      /* if (aux[1] & 512) flags |= ORC_TARGET_NEON_IWMMXT; */
      if (aux[1] & 4096) flags |= ORC_TARGET_NEON_NEON;
      if (aux[1] & 128) flags |= ORC_TARGET_ARM_EDSP;
#elif __aarch64__
      /**
       * Use HWCAP_ASIMD (1 << 1) to make sure Advanced SIMD (ASIMD) units exist in AArch64.
       * Note that some ARMv7 features including HWCAP_NEON are always supported by ARMv8 CPUs.
       */
      if (aux[1] & (1 << 1))
        flags |= ORC_TARGET_NEON_NEON | ORC_TARGET_ARM_EDSP; /** reuse 32bit flags */
#endif
      ORC_INFO("arm hwcap %08x", aux[1]);
    } if (aux[0] == AT_PLATFORM) {
      ORC_INFO("arm platform %s", (char *)aux[1]);
    } else if (aux[0] == AT_NULL) {
      break;
    }
  }

  close(fd);

  return flags;
}
#endif

static unsigned long
orc_cpu_arm_getflags_cpuinfo ()
{
  unsigned long ret = 0;

#if defined (_WIN32) && defined (_M_ARM64)
  /* On Windows, for desktop applications, we are on always on ARMv8 (aarch64)*/
  ret = ORC_TARGET_ARM_EDSP | ORC_TARGET_NEON_NEON;
#elif defined (__APPLE__) && defined (__arm64__) && TARGET_OS_OSX
  ret = ORC_TARGET_ARM_EDSP | ORC_TARGET_NEON_NEON;
#else
  char *cpuinfo;
  char *cpuinfo_line;
  char **flags;
  char **f;

  cpuinfo = get_proc_cpuinfo();
  if (cpuinfo == NULL) {
    ORC_DEBUG ("Failed to read /proc/cpuinfo");
    return 0;
  }

  cpuinfo_line = get_tag_value(cpuinfo, "CPU architecture");
  if (cpuinfo_line) {
    int arm_arch = strtoul (cpuinfo_line, NULL, 0);
    if (arm_arch >= 8L) {
      /* Armv8 always supports these, but they won't be listed
       * in the CPU info optional features */
      ret = ORC_TARGET_ARM_EDSP | ORC_TARGET_NEON_NEON;
      goto out;
    }

    free(cpuinfo_line);
  }

  cpuinfo_line = get_tag_value(cpuinfo, "Features");
  if (cpuinfo_line == NULL) {
    free (cpuinfo);
    return 0;
  }

  flags = strsplit(cpuinfo_line, ' ');
  for (f = flags; *f; f++) {
    if (strcmp (*f, "edsp") == 0)
      ret |= ORC_TARGET_ARM_EDSP;
    else if (strcmp (*f, "neon") == 0)
      ret |= ORC_TARGET_NEON_NEON;
    free (*f);
  }

  free (flags);

out:
  free (cpuinfo_line);
  free (cpuinfo);
#endif

  return ret;
}

unsigned long
orc_arm_get_cpu_flags (void)
{
  unsigned long neon_flags = 0;

#ifdef __linux__
  neon_flags = orc_check_neon_proc_auxv ();
#endif
  if (!neon_flags) {
    /* On ARM, /proc/self/auxv might not be accessible.
     * Fall back to /proc/cpuinfo */
    neon_flags = orc_cpu_arm_getflags_cpuinfo ();
  }

  if (orc_compiler_flag_check ("-neon")) {
    neon_flags &= ~ORC_TARGET_NEON_NEON;
  }

  return neon_flags;
}
#endif


