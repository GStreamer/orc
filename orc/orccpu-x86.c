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
#include <orc/orcsse.h>
#include <orc/orcmmx.h>
#include <orc/orcprogram.h>
#include <orc/orcutils.h>

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <time.h>

#if defined(__FreeBSD__) || defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#ifdef __sun
#include <sys/auxv.h>
#endif

/***** i386, amd64 *****/

#if defined(__sun)
#define USE_I386_GETISAX
#else
#define USE_I386_CPUID
#endif

int orc_x86_family_id;
int orc_x86_model_id;
int orc_x86_stepping;

#ifdef USE_I386_CPUINFO
static unsigned int
orc_sse_getflags_cpuinfo (char *cpuinfo)
{
  unsigned int sse_flags;
  char *cpuinfo_flags;
  char **flags;
  char **f;

  cpuinfo_flags = get_tag_value (cpuinfo, "flags");
  if (cpuinfo_flags == NULL) {
    free (cpuinfo);
    return;
  }

  flags = strsplit(cpuinfo_flags, ' ');
  for (f = flags; *f; f++) {
    if (strcmp (*f, "sse2") == 0) {
      ORC_DEBUG ("cpu flag %s", *f);
      sse_flags |= ORC_TARGET_SSE_SSE2;
    }
    if (strcmp (*f, "pni") == 0) {
      ORC_DEBUG ("cpu flag %s", *f);
      sse_flags |= ORC_TARGET_SSE_SSE3;
    }
    if (strcmp (*f, "ssse3") == 0) {
      ORC_DEBUG ("cpu flag %s", *f);
      sse_flags |= ORC_TARGET_SSE_SSSE3;
    }
    if (strcmp (*f, "sse4_1") == 0) {
      ORC_DEBUG ("cpu flag %s", *f);
      sse_flags |= ORC_TARGET_SSE_SSSE4_1;
    }
    if (strcmp (*f, "sse4_2") == 0) {
      ORC_DEBUG ("cpu flag %s", *f);
      sse_flags |= ORC_TARGET_SSE_SSSE4_2;
    }
    if (strcmp (*f, "sse4a") == 0) {
      ORC_DEBUG ("cpu flag %s", *f);
      sse_flags |= ORC_TARGET_SSE_SSSE4A;
    }
    if (strcmp (*f, "sse5") == 0) {
      ORC_DEBUG ("cpu flag %s", *f);
      orc_cpu_flags |= ORC_CPU_FLAG_SSE5;
    }

    free (*f);
  }
  free (flags);
  free (cpuinfo);
  free (cpuinfo_flags);

  return sse_flags;
}

static unsigned int
orc_mmx_getflags_cpuinfo (char *cpuinfo)
{
  unsigned int sse_flags;
  char *cpuinfo_flags;
  char **flags;
  char **f;

  cpuinfo_flags = get_tag_value (cpuinfo, "flags");
  if (cpuinfo_flags == NULL) {
    free (cpuinfo);
    return;
  }

  flags = strsplit(cpuinfo_flags, ' ');
  for (f = flags; *f; f++) {
    if (strcmp (*f, "mmx") == 0) {
      ORC_DEBUG ("cpu flag %s", *f);
      sse_flags |= ORC_TARGET_MMX_MMX;
    }
    if (strcmp (*f, "mmxext") == 0) {
      ORC_DEBUG ("cpu flag %s", *f);
      sse_flags |= ORC_TARGET_MMX_MMXEXT;
    }
    if (strcmp (*f, "ssse3") == 0) {
      ORC_DEBUG ("cpu flag %s", *f);
      sse_flags |= ORC_TARGET_MMX_SSSE3;
    }
    if (strcmp (*f, "3dnow") == 0) {
      ORC_DEBUG ("cpu flag %s", *f);
      sse_flags |= ORC_TARGET_MMX_3DNOW;
    }
    if (strcmp (*f, "3dnowext") == 0) {
      ORC_DEBUG ("cpu flag %s", *f);
      sse_flags |= ORC_TARGET_MMX_3DNOWEXT;
    }

    free (*f);
  }
  free (flags);
  free (cpuinfo);
  free (cpuinfo_flags);

  return sse_flags;
}
#endif

#ifdef USE_I386_CPUID
#ifdef _MSC_VER
static void
get_cpuid (orc_uint32 op, orc_uint32 init_ecx, orc_uint32 *a, orc_uint32 *b, orc_uint32 *c, orc_uint32 *d)
{
  int tmp[4];
  __cpuid(tmp, op);
  *a = tmp[0];
  *b = tmp[1];
  *c = tmp[2];
  *d = tmp[3];
}
#endif

#ifdef __i386__
static void
get_cpuid (orc_uint32 op, orc_uint32 init_ecx, orc_uint32 *a, orc_uint32 *b,
    orc_uint32 *c, orc_uint32 *d)
{
  *a = op;
  *c = init_ecx;
  __asm__ (
      "  pushl %%ebx\n"
      "  cpuid\n"
      "  mov %%ebx, %%esi\n"
      "  popl %%ebx\n"
      : "+a" (*a), "=S" (*b), "+c" (*c), "=d" (*d));
}
#endif

#ifdef __amd64__
static void
get_cpuid (orc_uint32 op, orc_uint32 init_ecx, orc_uint32 *a, orc_uint32 *b,
    orc_uint32 *c, orc_uint32 *d)
{
  *a = op;
  *c = init_ecx;
  __asm__ (
      "  cpuid\n"
      : "+a" (*a), "=b" (*b), "+c" (*c), "=d" (*d));
  //ORC_ERROR("cpuid %08x %08x -> %08x %08x %08x %08x", op, init_ecx, *a, *b, *c, *d);
}
#endif

#if 0
static void
test_cpuid (void *ignored)
{
  orc_uint32 eax, ebx, ecx, edx;

  get_cpuid (0x00000000, &eax, &ebx, &ecx, &edx);
}
#endif

struct desc_struct {
  int desc;
  int level;
  int size;
};
struct desc_struct cache_descriptors[] = {
  { 0x0a, 1, 8*1024 },
  { 0x0c, 1, 16*1024 },
  { 0x0d, 1, 16*1024 },
  { 0x0e, 1, 24*1024 },
  { 0x21, 2, 256*1024 },
  { 0x22, 3, 512*1024 },
  { 0x23, 3, 1024*1024 },
  { 0x25, 3, 2*1024*1024 },
  { 0x29, 3, 4*1024*1024 },
  { 0x2c, 1, 32*1024 },
  { 0x41, 2, 128*1024 },
  { 0x42, 2, 256*1024 },
  { 0x43, 2, 512*1024 },
  { 0x44, 2, 1*1024*1024 },
  { 0x45, 2, 2*1024*1024 },
  { 0x46, 3, 4*1024*1024 },
  { 0x47, 3, 8*1024*1024 },
  { 0x48, 2, 3*1024*1024 },
  { 0x49, 2, 4*1024*1024 }, /* special case */
  { 0x4a, 3, 6*1024*1024 },
  { 0x4b, 3, 8*1024*1024 },
  { 0x4c, 3, 12*1024*1024 },
  { 0x4d, 3, 16*1024*1024 },
  { 0x4e, 2, 6*1024*1024 },
  { 0x60, 1, 16*1024 },
  { 0x66, 1, 8*1024 },
  { 0x67, 1, 16*1024 },
  { 0x68, 1, 32*1024 },
  { 0x78, 2, 1*1024*1024 },
  { 0x79, 2, 128*1024 },
  { 0x7a, 2, 256*1024 },
  { 0x7b, 2, 512*1024 },
  { 0x7c, 2, 1*1024*1024 },
  { 0x7d, 2, 2*1024*1024 },
  { 0x7f, 2, 512*1024 },
  { 0x80, 2, 512*1024 },
  { 0x82, 2, 256*1024 },
  { 0x83, 2, 512*1024 },
  { 0x84, 2, 1*1024*1024 },
  { 0x85, 2, 2*1024*1024 },
  { 0x86, 2, 512*1024 },
  { 0x87, 2, 1*1024*1024 },
  { 0xe4, 3, 8*1024*1024 }
};

static void
handle_cache_descriptor (unsigned int desc)
{
  int i;

  if (desc == 0) return;

  /* special case */
  if (desc == 0x49 && orc_x86_family_id == 0xf && orc_x86_model_id == 0x6) {
    ORC_DEBUG("level %d size %d", 3, 4*1024*1024);
    _orc_data_cache_size_level3 = 4*1024*1024;
    return;
  }

  for(i=0;i<sizeof(cache_descriptors)/sizeof(cache_descriptors[0]);i++){
    if (desc == cache_descriptors[i].desc) {
      ORC_DEBUG("level %d size %d", cache_descriptors[i].level,
          cache_descriptors[i].size);
      switch (cache_descriptors[i].level) {
        case 1:
          _orc_data_cache_size_level1 = cache_descriptors[i].size;
          break;
        case 2:
          _orc_data_cache_size_level2 = cache_descriptors[i].size;
          break;
        case 3:
          _orc_data_cache_size_level3 = cache_descriptors[i].size;
          break;
      }
    }
  }
}

static unsigned int
orc_sse_detect_cpuid (void)
{
  orc_uint32 eax, ebx, ecx, edx;
  orc_uint32 level;
  orc_uint32 vendor;
  unsigned int sse_flags = 0;
#if 0
  int ret;

  orc_fault_check_enable ();
  ret = orc_fault_check_try(test_cpuid, NULL);
  orc_fault_check_disable ();
  if (!ret) {
    /* CPU thinks cpuid is an illegal instruction. */
    return;
  }
#endif

  get_cpuid (0x00000000, 0, &level, &vendor, &ecx, &edx);

  ORC_DEBUG("cpuid %d %08x %08x %08x", level, vendor, ecx, edx);

  if (level >= 1) {
    get_cpuid (0x00000001, 0, &eax, &ebx, &ecx, &edx);

    /* generic flags */
    if (edx & (1<<26)) {
      sse_flags |= ORC_TARGET_SSE_SSE2;
    }
    if (ecx & (1<<0)) {
      sse_flags |= ORC_TARGET_SSE_SSE3;
    }
    if (ecx & (1<<9)) {
      sse_flags |= ORC_TARGET_SSE_SSSE3;
    }
    if (ecx & (1<<19)) {
      sse_flags |= ORC_TARGET_SSE_SSE4_1;
    }
    if (ecx & (1<<20)) {
      sse_flags |= ORC_TARGET_SSE_SSE4_2;
    }

    if (vendor == (('G'<<0)|('e'<<8)|('n'<<16)|('u'<<24))) {
      int family_id = (eax>>8)&0xf;
      int model_id = (eax>>4)&0xf;
      int ext_family_id = (eax>>20)&0xff;
      int ext_model_id = (eax>>16)&0xf;

      if (family_id == 0xf) {
        orc_x86_family_id = family_id + ext_family_id;
      } else {
        orc_x86_family_id = family_id;
      }

      if (model_id == 0xf) {
        orc_x86_model_id = model_id + ext_model_id;
      } else {
        orc_x86_model_id = model_id;
      }

      orc_x86_stepping = eax&0xf;

      ORC_DEBUG("family_id %d model_id %d stepping %d",
          orc_x86_family_id, orc_x86_model_id, orc_x86_stepping);

    }
  }

  if (level >= 2 && vendor == (('G'<<0)|('e'<<8)|('n'<<16)|('u'<<24))) {
    get_cpuid (0x00000002, 0, &eax, &ebx, &ecx, &edx);

    if ((eax&0x80000000) == 0) {
      handle_cache_descriptor ((eax>>8)&0xff);
      handle_cache_descriptor ((eax>>16)&0xff);
      handle_cache_descriptor ((eax>>24)&0xff);
    }
    if ((ebx&0x80000000) == 0) {
      handle_cache_descriptor (ebx&0xff);
      handle_cache_descriptor ((ebx>>8)&0xff);
      handle_cache_descriptor ((ebx>>16)&0xff);
      handle_cache_descriptor ((ebx>>24)&0xff);
    }
    if ((ecx&0x80000000) == 0) {
      handle_cache_descriptor (ecx&0xff);
      handle_cache_descriptor ((ecx>>8)&0xff);
      handle_cache_descriptor ((ecx>>16)&0xff);
      handle_cache_descriptor ((ecx>>24)&0xff);
    }
    if ((edx&0x80000000) == 0) {
      handle_cache_descriptor (edx&0xff);
      handle_cache_descriptor ((edx>>8)&0xff);
      handle_cache_descriptor ((edx>>16)&0xff);
      handle_cache_descriptor ((edx>>24)&0xff);
    }
  }

  if (level >= 4 && vendor == (('G'<<0)|('e'<<8)|('n'<<16)|('u'<<24))) {
    int i;
    for(i=0;i<10;i++){
      int type;
      int level;
      int l;
      int p;
      int w;
      int s;

      get_cpuid (0x00000004, i, &eax, &ebx, &ecx, &edx);
      type = eax&0xf;
      if (type == 0) break;

      level = (eax>>5)&0x7;
      l = ((ebx>>0)&0xfff)+1;
      p = ((ebx>>12)&0x3ff)+1;
      w = ((ebx>>22)&0x3ff)+1;
      s = ecx + 1;

      ORC_DEBUG("type %d level %d line size %d partitions %d ways %d sets %d",
          type, level, l, p, w, s);
      if (type == 1 || type == 3) {
        switch (level) {
          case 1:
            _orc_data_cache_size_level1 = l*p*w*s;
            break;
          case 2:
            _orc_data_cache_size_level2 = l*p*w*s;
            break;
          case 3:
            _orc_data_cache_size_level3 = l*p*w*s;
            break;
        }
      }
    }

  }
  
  if (level >= 1 && vendor == (('A'<<0)|('u'<<8)|('t'<<16)|('h'<<24))) {
    get_cpuid (0x80000001, 0, &eax, &ebx, &ecx, &edx);

    /* AMD flags */
    if (ecx & (1<<6)) {
      sse_flags |= ORC_TARGET_SSE_SSE4A;
    }
    if (ecx & (1<<11)) {
      sse_flags |= ORC_TARGET_SSE_SSE5;
    }

    get_cpuid (0x80000005, 0, &eax, &ebx, &ecx, &edx);
    _orc_data_cache_size_level1 = ((ecx>>24)&0xff) * 1024;
    ORC_DEBUG ("L1 D-cache: %d kbytes, %d-way, %d lines/tag, %d line size",
        (ecx>>24)&0xff, (ecx>>16)&0xff, (ecx>>8)&0xff, ecx&0xff);
    ORC_DEBUG ("L1 I-cache: %d kbytes, %d-way, %d lines/tag, %d line size",
        (edx>>24)&0xff, (edx>>16)&0xff, (edx>>8)&0xff, edx&0xff);

    get_cpuid (0x80000006, 0, &eax, &ebx, &ecx, &edx);
    _orc_data_cache_size_level2 = ((ecx>>16)&0xffff) * 1024;
    ORC_DEBUG ("L2 cache: %d kbytes, %d assoc, %d lines/tag, %d line size",
        (ecx>>16)&0xffff, (ecx>>12)&0xf, (ecx>>8)&0xf, ecx&0xff);
  }

  if (orc_compiler_flag_check ("-sse2")) {
    sse_flags &= ~ORC_TARGET_SSE_SSE2;
  }
  if (orc_compiler_flag_check ("-sse3")) {
    sse_flags &= ~ORC_TARGET_SSE_SSE3;
  }
  if (orc_compiler_flag_check ("-ssse3")) {
    sse_flags &= ~ORC_TARGET_SSE_SSSE3;
  }
  if (orc_compiler_flag_check ("-sse41")) {
    sse_flags &= ~ORC_TARGET_SSE_SSE4_1;
  }
  if (orc_compiler_flag_check ("-sse42")) {
    sse_flags &= ~ORC_TARGET_SSE_SSE4_2;
  }
  if (orc_compiler_flag_check ("-sse4a")) {
    sse_flags &= ~ORC_TARGET_SSE_SSE4A;
  }
  if (orc_compiler_flag_check ("-sse5")) {
    sse_flags &= ~ORC_TARGET_SSE_SSE5;
  }

  return sse_flags;
}

static unsigned int
orc_mmx_detect_cpuid (void)
{
  orc_uint32 eax, ebx, ecx, edx;
  orc_uint32 level;
  char vendor[13] = { 0 };
  unsigned int mmx_flags = 0;

  get_cpuid (0x00000000, 0, &level, (orc_uint32 *)(vendor+0),
      (orc_uint32 *)(vendor+8), (orc_uint32 *)(vendor+4));

  ORC_DEBUG("cpuid %d %s", level, vendor);

  if (level < 1) {
    return 0;
  }

  get_cpuid (0x00000001, 0, &eax, &ebx, &ecx, &edx);

  /* Intel flags */
  if (edx & (1<<23)) {
    mmx_flags |= ORC_TARGET_MMX_MMX;
  }
  if (ecx & (1<<9)) {
    mmx_flags |= ORC_TARGET_MMX_SSSE3;
  }
  
  if (memcmp (vendor, "AuthenticAMD", 12) == 0) {
    get_cpuid (0x80000001, 0, &eax, &ebx, &ecx, &edx);

    /* AMD flags */
    if (edx & (1<<22)) {
      mmx_flags |= ORC_TARGET_MMX_MMXEXT;
    }
    if (edx & (1<<31)) {
      mmx_flags |= ORC_TARGET_MMX_3DNOW;
    }
    if (edx & (1<<30)) {
      mmx_flags |= ORC_TARGET_MMX_3DNOWEXT;
    }
  }

  return mmx_flags;
}
#endif

#ifdef USE_I386_GETISAX
static unsigned int
orc_sse_detect_getisax (void)
{
  unsigned int sse_flags;
  uint_t ui;

  getisax (&ui, 1);

  if (ui & AV_386_SSE2) {
     sse_flags |= ORC_TARGET_SSE_SSE2;
  }
  if (ui & AV_386_SSE3) {
     sse_flags |= ORC_TARGET_SSE_SSE3;
  }

  /* guesses.  if these fail to compile, please fix */
  if (ui & AV_386_SSSE3) {
     sse_flags |= ORC_TARGET_SSE_SSSE3;
  }
  if (ui & AV_386_SSE4_1) {
     sse_flags |= ORC_TARGET_SSE_SSE4_1;
  }
  if (ui & AV_386_SSE4_2) {
     sse_flags |= ORC_TARGET_SSE_SSE4_2;
  }
  if (ui & AV_386_AMD_SSE4A) {
     sse_flags |= ORC_TARGET_SSE_SSE4A;
  }
//  if (ui & AV_386_SSE5) {
//     sse_flags |= ORC_TARGET_SSE_SSE5;
//  }

  return sse_flags;
}

static unsigned int
orc_mmx_detect_getisax (void)
{
  unsigned int mmx_flags;
  uint_t ui;

  getisax (&ui, 1);

  if (ui & AV_386_MMX) {
     mmx_flags |= ORC_TARGET_MMX_MMX;
  }

  /* guesses.  if these fail to compile, please fix */
  if (ui & AV_386_AMD_MMX) {
     mmx_flags |= ORC_TARGET_MMX_MMXEXT;
  }
  if (ui & AV_386_SSSE3) {
     mmx_flags |= ORC_TARGET_MMX_SSSE3;
  }
  if (ui & AV_386_AMD_3DNow) {
     mmx_flags |= ORC_TARGET_MMX_3DNOW;
  }
  if (ui & AV_386_AMD_3DNowx) {
     mmx_flags |= ORC_TARGET_MMX_3DNOWEXT;
  }

  return mmx_flags;
}
#endif

#if 0
/* Reduce the set of CPU capabilities detected by whatever detection mechanism
 * was chosen, according to kernel limitations.  SSE requires kernel support for
 * use.
 */
static void
orc_cpu_detect_kernel_support (void)
{
#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__APPLE__)
  int ret, enabled;
  size_t len;

  len = sizeof(enabled);
  ret = sysctlbyname("hw.instruction_sse", &enabled, &len, NULL, 0);
  if (ret || !enabled) {
    orc_cpu_flags &= ~(ORC_CPU_FLAG_SSE | ORC_CPU_FLAG_SSE2 |
		       ORC_CPU_FLAG_MMXEXT | ORC_CPU_FLAG_SSE3);
  }
#elif defined(__linux__)
  /*
   * Might also want to grow a check for the old RedHat + Linux 2.2
   * unmasked SSE FPU exception bug.  Other than that, if /proc/cpuinfo
   * reported SSE, then it's safe.
   */
#elif defined(__sun) || defined(__NetBSD__) || defined(__OpenBSD__)
  /* Solaris/NetBSD/OpenBSD are OK */
#else
   
  ORC_WARNING("Operating system is not known to support SSE.  "
      "Assuming it does, which might cause problems");
#if 0
  orc_cpu_flags &= ~(ORC_CPU_FLAG_SSE | ORC_CPU_FLAG_SSE2 |
		     ORC_CPU_FLAG_MMXEXT | ORC_CPU_FLAG_SSE3);
#endif
#endif
}
#endif

unsigned int
orc_sse_get_cpu_flags(void)
{
  //orc_cpu_detect_kernel_support ();

#ifdef USE_I386_CPUID
  return orc_sse_detect_cpuid ();
#endif
#ifdef USE_I386_GETISAX
  return orc_sse_detect_getisax ();
#endif
#ifdef USE_I386_CPUINFO
  return orc_sse_detect_cpuinfo ();
#endif
}

unsigned int
orc_mmx_get_cpu_flags(void)
{
  //orc_cpu_detect_kernel_support ();

#ifdef USE_I386_CPUID
  return orc_mmx_detect_cpuid ();
#endif
#ifdef USE_I386_GETISAX
  return orc_mmx_detect_getisax ();
#endif
#ifdef USE_I386_CPUINFO
  return orc_mmx_detect_cpuinfo ();
#endif
}



