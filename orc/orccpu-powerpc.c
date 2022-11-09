/*
 * LIBORC - Library of Optimized Inner Loops
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
#include <orc/orc.h>
#include <orc/orcinternal.h>

#if defined(__linux__)
#include <linux/auxvec.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#ifndef PPC_FEATURE_HAS_ALTIVEC
/* From linux-2.6/include/asm-powerpc/cputable.h */
#define PPC_FEATURE_HAS_ALTIVEC 0x10000000
#endif

#ifndef PPC_FEATURE_HAS_VSX
#define PPC_FEATURE_HAS_VSX 0x00000080
#endif

#ifndef PPC_FEATURE2_ARCH_2_07
#define PPC_FEATURE2_ARCH_2_07 0x80000000
#endif

#endif

#if defined(__FreeBSD__) || defined(__APPLE__) || defined(__NetBSD__)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#if defined(__OpenBSD__)
#include <sys/param.h>
#include <sys/sysctl.h>
#include <machine/cpu.h>
#endif

/***** powerpc *****/
int orc_powerpc_cpu_flags;

#if 0
static unsigned long
orc_profile_stamp_tb(void)
{
  unsigned long ts;
  __asm__ __volatile__("mftb %0\n" : "=r" (ts));
  return ts;
}
#endif

#if !defined(__FreeBSD__) && !defined(__FreeBSD_kernel__) && !defined(__OpenBSD__) && !defined(__APPLE__) && !defined(__linux__)
static void
test_altivec (void * ignored)
{
  asm volatile ("vor v0, v0, v0\n");
}
#endif

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__NetBSD__)
#if defined(__APPLE__)
#define SYSCTL "hw.vectorunit"
#elif defined(__NetBSD__)
#define SYSCTL "machdep.altivec"
#else
#define SYSCTL "hw.altivec"
#endif

static void
orc_check_powerpc_sysctl_bsd (void)
{
  int ret, vu;
  size_t len;

  len = sizeof(vu);
  ret = sysctlbyname(SYSCTL, &vu, &len, NULL, 0);
  if (!ret && vu) {
    orc_powerpc_cpu_flags |= ORC_TARGET_POWERPC_ALTIVEC;
  }
}
#endif

#if defined(__OpenBSD__)
static void
orc_check_powerpc_sysctl_openbsd (void)
{
  int mib[2], ret, vu;
  size_t len;

  mib[0] = CTL_MACHDEP;
  mib[1] = CPU_ALTIVEC;

  len = sizeof(vu);
  ret = sysctl(mib, 2, &vu, &len, NULL, 0);
  if (!ret && vu) {
    orc_powerpc_cpu_flags |= ORC_TARGET_POWERPC_ALTIVEC;
  }
}
#endif

#if defined(__linux__)
static void
orc_check_powerpc_proc_auxv (void)
{
  unsigned long buf[64];
  ssize_t count;
  int fd, i, found = 0;

  fd = open("/proc/self/auxv", O_RDONLY);
  if (fd < 0)
    return;

  while (1) {
    count = read(fd, buf, sizeof(buf));
    if (count <= 0)
      break;

    for (i=0; i < (count / sizeof(unsigned long)); i += 2) {
      if (buf[i] == AT_NULL)
        break;
      if (buf[i] == AT_HWCAP) {
        if (buf[i + 1] & PPC_FEATURE_HAS_ALTIVEC)
          orc_powerpc_cpu_flags |= ORC_TARGET_POWERPC_ALTIVEC;
        if (buf[i + 1] & PPC_FEATURE_HAS_VSX)
          orc_powerpc_cpu_flags |= ORC_TARGET_POWERPC_VSX;
        found++;
      }
      if (buf[i] == AT_HWCAP2) {
        if (buf[i + 1] & PPC_FEATURE2_ARCH_2_07)
          orc_powerpc_cpu_flags |= ORC_TARGET_POWERPC_V207;
        found++;
      }
      if (buf[i] == AT_PLATFORM) {
        _orc_cpu_name = (char*)buf[i + 1];
        found++;
      }
#ifdef AT_L1D_CACHESIZE
      if (buf[i] == AT_L1D_CACHESIZE) {
        _orc_data_cache_size_level1 = buf[i + 1];
        found++;
      }
#endif
#ifdef AT_L2_CACHESIZE
      if (buf[i] == AT_L2_CACHESIZE) {
        _orc_data_cache_size_level2 = buf[i + 1];
        found++;
      }
#endif
#ifdef AT_L3_CACHESIZE
      if (buf[i] == AT_L3_CACHESIZE) {
        _orc_data_cache_size_level3 = buf[i + 1];
        found++;
      }
#endif
      if (found == 6)
        break;
    }
  }

  close(fd);
}
#endif

#if !defined(__FreeBSD__) && !defined(__FreeBSD_kernel__) && !defined(__OpenBSD__) && !defined(__APPLE__) && !defined(__linux__) && !defined(__NetBSD__)
static void
orc_check_powerpc_fault (void)
{
  orc_fault_check_enable ();
  if (orc_fault_check_try(test_altivec, NULL)) {
    ORC_DEBUG ("cpu flag altivec");
    orc_powerpc_cpu_flags |= ORC_TARGET_POWERPC_ALTIVEC;
  }
  orc_fault_check_disable ();
}
#endif

void
powerpc_detect_cpu_flags (void)
{
  static int inited = 0;

  if (inited) return;
  inited = 1;

#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__APPLE__) || defined(__NetBSD__)
  orc_check_powerpc_sysctl_bsd();
#elif defined(__OpenBSD__)
  orc_check_powerpc_sysctl_openbsd();
#elif defined(__linux__)
  orc_check_powerpc_proc_auxv();
#else
  orc_check_powerpc_fault();
#endif

  if (orc_compiler_flag_check("-altivec")) {
    orc_powerpc_cpu_flags &= ~ORC_TARGET_POWERPC_ALTIVEC;
  }
  if (orc_compiler_flag_check("-vsx")) {
    orc_powerpc_cpu_flags &= ~ORC_TARGET_POWERPC_VSX;
  }
  if (orc_compiler_flag_check("-v207")) {
    orc_powerpc_cpu_flags &= ~ORC_TARGET_POWERPC_V207;
  }
}
