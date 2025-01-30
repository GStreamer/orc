/*
 * ORC - Library of Optimized Inner Loops
 * Copyright (c) 2003,2004,2010 David A. Schleef <ds@schleef.org>
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

#include <fcntl.h>
#include <immintrin.h>

#ifdef _MSC_VER
#  include <intrin.h>
#elif defined(__GNUC__)
#if __GNUC__ >= 8 && __GNUC__ < 9
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=71659
#  include <xsaveintrin.h>
#endif
#endif

#if !defined(_MSC_VER) || defined(__clang__)
#define ORC_TARGET_XSAVE __attribute__((target("xsave")))
#else
#define ORC_TARGET_XSAVE
#endif

#include <orc/orcdebug.h>
#include <orc/orcx86.h>
#include <orc/orcx86insn.h>
#include <orc/orcx86-private.h>
#include <orc/orcsse.h>
#include <orc/orcmmx.h>
#include <orc/orcprogram.h>
#include <orc/orcutils.h>

#include "orcinternal.h"

static char orc_x86_cpu_string[49];
static int orc_x86_cpu_microarchitecture;

#if defined(_MSC_VER)
void
orc_x86_cpu_get_cpuid (orc_uint32 op, orc_uint32 *a, orc_uint32 *b, orc_uint32 *c, orc_uint32 *d)
{
  int tmp[4];
  __cpuid(tmp, op);
  *a = tmp[0];
  *b = tmp[1];
  *c = tmp[2];
  *d = tmp[3];
}

static void
get_cpuid_ecx (orc_uint32 op, orc_uint32 init_ecx, orc_uint32 *a, orc_uint32 *b, orc_uint32 *c, orc_uint32 *d)
{
#if _MSC_VER >= 1500
  int tmp[4];
  __cpuidex(tmp, op, init_ecx);
  *a = tmp[0];
  *b = tmp[1];
  *c = tmp[2];
  *d = tmp[3];
#else
  *a = 0;
  *b = 0;
  *c = 0;
  *d = 0;
#endif
}

#elif defined(__GNUC__) || defined(__clang__) || defined (__SUNPRO_C)

static void
get_cpuid_ecx (orc_uint32 op, orc_uint32 init_ecx, orc_uint32 *a, orc_uint32 *b,
    orc_uint32 *c, orc_uint32 *d)
{
  *a = op;
  *c = init_ecx;
#if defined(HAVE_I386)
  __asm__ (
      "  pushl %%ebx\n"
      "  cpuid\n"
      "  mov %%ebx, %%esi\n"
      "  popl %%ebx\n"
      : "+a" (*a), "=S" (*b), "+c" (*c), "=d" (*d));
#elif defined(HAVE_AMD64)
  __asm__ (
      "  cpuid\n"
      : "+a" (*a), "=b" (*b), "+c" (*c), "=d" (*d));
#endif
}

void
orc_x86_cpu_get_cpuid (orc_uint32 op, orc_uint32 *a, orc_uint32 *b,
    orc_uint32 *c, orc_uint32 *d)
{
  get_cpuid_ecx (op, 0, a, b, c, d);
}

#else

/* FIXME generate a orc_x86_cpu_get_cpuid() function at runtime. */
#error Need orc_x86_cpu_get_cpuid() function.

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
  if (desc == 0x49 && _orc_cpu_family == 0xf && _orc_cpu_model == 0x6) {
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

static void
orc_x86_cpuid_get_branding_string (void)
{
  orc_x86_cpu_get_cpuid (0x80000002,
      (orc_uint32 *)(orc_x86_cpu_string+0),
      (orc_uint32 *)(orc_x86_cpu_string+4),
      (orc_uint32 *)(orc_x86_cpu_string+8),
      (orc_uint32 *)(orc_x86_cpu_string+12));
  orc_x86_cpu_get_cpuid (0x80000003,
      (orc_uint32 *)(orc_x86_cpu_string+16),
      (orc_uint32 *)(orc_x86_cpu_string+20),
      (orc_uint32 *)(orc_x86_cpu_string+24),
      (orc_uint32 *)(orc_x86_cpu_string+28));
  orc_x86_cpu_get_cpuid (0x80000004,
      (orc_uint32 *)(orc_x86_cpu_string+32),
      (orc_uint32 *)(orc_x86_cpu_string+36),
      (orc_uint32 *)(orc_x86_cpu_string+40),
      (orc_uint32 *)(orc_x86_cpu_string+44));

  ORC_INFO ("processor string '%s'", orc_x86_cpu_string);

  _orc_cpu_name = orc_x86_cpu_string;
}

#ifdef ORC_NEEDS_ASM_XSAVE
static orc_uint32
get_xcr0 (void)
{
  orc_uint32 xcr0;
  __asm__ ("xgetbv" : "=a" (xcr0) : "c" (0) : "%edx" );
  return xcr0;
}
#else
static orc_uint32 ORC_TARGET_XSAVE
get_xcr0 (void)
{
  return _xgetbv(0);
}
#endif

static void
orc_x86_cpuid_handle_family_model_stepping (void)
{
  orc_uint32 eax, ebx, ecx, edx;
  int family_id;
  int model_id;
  int ext_family_id;
  int ext_model_id;

  orc_x86_cpu_get_cpuid (0x00000001, &eax, &ebx, &ecx, &edx);

  family_id = (eax>>8)&0xf;
  model_id = (eax>>4)&0xf;
  ext_family_id = (eax>>20)&0xff;
  ext_model_id = (eax>>16)&0xf;

  _orc_cpu_family = family_id + ext_family_id;
  _orc_cpu_model = (ext_model_id << 4) | model_id;
  _orc_cpu_stepping = eax&0xf;

  ORC_INFO ("family_id %d model_id %d stepping %d",
      _orc_cpu_family, _orc_cpu_model, _orc_cpu_stepping);
}

static void
orc_x86_cpu_generic_detect (orc_uint32 level)
{
  if (level >= 1) {
    orc_x86_cpuid_handle_family_model_stepping ();
  }
}

static void
orc_x86_cpu_intel_detect (orc_uint32 level)
{
  orc_uint32 eax, ebx, ecx, edx;

  if (level >= 1) {

    orc_x86_cpuid_handle_family_model_stepping ();

    orc_x86_cpu_microarchitecture = ORC_X86_UNKNOWN;
    if (_orc_cpu_family == 6) {
      switch (_orc_cpu_model) {
        case 6: /* Mendocino */
        case 11: /* Tualatin-256 */
          orc_x86_cpu_microarchitecture = ORC_X86_P6;
          break;
        case 15:
        case 22:
          orc_x86_cpu_microarchitecture = ORC_X86_CORE;
          break;
        case 23:
        case 29:
          orc_x86_cpu_microarchitecture = ORC_X86_PENRYN;
          break;
        case 26:
          orc_x86_cpu_microarchitecture = ORC_X86_NEHALEM;
          break;
        case 28:
          orc_x86_cpu_microarchitecture = ORC_X86_BONNELL;
          break;
          /* orc_x86_cpu_microarchitecture = ORC_X86_WESTMERE; */
          /* orc_x86_cpu_microarchitecture = ORC_X86_SANDY_BRIDGE; */
      }
    } else if (_orc_cpu_family == 15) {
      orc_x86_cpu_microarchitecture = ORC_X86_NETBURST;
    }
  }

  if (level >= 2) {
    orc_x86_cpu_get_cpuid (0x00000002, &eax, &ebx, &ecx, &edx);

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

  if (level >= 4) {
    int i;
    for(i=0;i<10;i++){
      int type;
      int level;
      int l;
      int p;
      int w;
      int s;

      get_cpuid_ecx (0x00000004, i, &eax, &ebx, &ecx, &edx);
      type = eax&0xf;
      if (type == 0) break;

      level = (eax>>5)&0x7;
      l = ((ebx>>0)&0xfff)+1;
      p = ((ebx>>12)&0x3ff)+1;
      w = ((ebx>>22)&0x3ff)+1;
      s = ecx + 1;

      ORC_INFO ("type %d level %d line size %d partitions %d ways %d sets %d",
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

  orc_x86_cpu_get_cpuid (0x80000000, &level, &ebx, &ecx, &edx);

  if (level >= 4) {
    orc_x86_cpuid_get_branding_string ();
  }
}
  
static void
orc_x86_cpu_amd_detect (orc_uint32 level)
{
  orc_uint32 eax = 0;
  orc_uint32 ebx = 0;
  orc_uint32 ecx = 0;
  orc_uint32 edx = 0;

  if (level >= 1) {
    orc_x86_cpuid_handle_family_model_stepping ();

    orc_x86_cpu_microarchitecture = ORC_X86_UNKNOWN;
    switch (_orc_cpu_family) {
      case 5:
        /* Don't know if 8 is correct */
        if (_orc_cpu_model < 8) {
          orc_x86_cpu_microarchitecture = ORC_X86_K5;
        } else {
          orc_x86_cpu_microarchitecture = ORC_X86_K6;
        }
        break;
      case 6:
        orc_x86_cpu_microarchitecture = ORC_X86_K7;
        break;
      case 0xf:
        orc_x86_cpu_microarchitecture = ORC_X86_K8;
        break;
      case 0x10:
        orc_x86_cpu_microarchitecture = ORC_X86_K10;
        break;
      default:
        break;
    }
  }

  orc_x86_cpu_get_cpuid (0x80000000, &level, &ebx, &ecx, &edx);

  if (level >= 4) {
    orc_x86_cpuid_get_branding_string ();
  }

  if (level >= 6) {
    orc_x86_cpu_get_cpuid (0x80000005, &eax, &ebx, &ecx, &edx);

    _orc_data_cache_size_level1 = ((ecx>>24)&0xff) * 1024;
    ORC_INFO ("L1 D-cache: %d kbytes, %d-way, %d lines/tag, %d line size",
        (ecx>>24)&0xff, (ecx>>16)&0xff, (ecx>>8)&0xff, ecx&0xff);
    ORC_INFO ("L1 I-cache: %d kbytes, %d-way, %d lines/tag, %d line size",
        (edx>>24)&0xff, (edx>>16)&0xff, (edx>>8)&0xff, edx&0xff);

    orc_x86_cpu_get_cpuid (0x80000006, &eax, &ebx, &ecx, &edx);
    _orc_data_cache_size_level2 = ((ecx>>16)&0xffff) * 1024;
    ORC_INFO ("L2 cache: %d kbytes, %d assoc, %d lines/tag, %d line size",
        (ecx>>16)&0xffff, (ecx>>12)&0xf, (ecx>>8)&0xf, ecx&0xff);
  }
}

void
orc_x86_cpu_detect (orc_uint32 *level, OrcX86CPUVendor *vendor)
{
  static OrcX86CPUVendor orc_x86_cpu_vendor = 0;
  static orc_uint32 orc_x86_cpu_level = 0;
  static int inited = 0;
  orc_uint32 ebx = 0;
  orc_uint32 edx = 0;
  orc_uint32 vendor_id = 0;

  if (inited)
    goto done;

  inited = 1;

  orc_x86_cpu_get_cpuid (0x00000000, &orc_x86_cpu_level, &ebx, &vendor_id, &edx);

  ORC_DEBUG("cpuid %d %08x %08x %08x", orc_x86_cpu_level, ebx, edx, vendor_id);

#define ORC_X86_GenuineIntel (('n'<<0)|('t'<<8)|('e'<<16)|('l'<<24))
#define ORC_X86_AuthenticAMD (('c'<<0)|('A'<<8)|('M'<<16)|('D'<<24))
#define ORC_X86_HygonGenuine (('u'<<0)|('i'<<8)|('n'<<16)|('e'<<24))
#define ORC_X86_CentaurHauls (('a'<<0)|('u'<<8)|('l'<<16)|('s'<<24))
#define ORC_X86_CyrixInstead (('t'<<0)|('e'<<8)|('a'<<16)|('d'<<24))
#define ORC_X86_GenuineTMx86 (('M'<<0)|('x'<<8)|('8'<<16)|('6'<<24))
#define ORC_X86_Geode_by_NSC ((' '<<0)|('N'<<8)|('S'<<16)|('6'<<24))
#define ORC_X86_NexGenDriven (('i'<<0)|('v'<<8)|('e'<<16)|('n'<<24))
#define ORC_X86_RiseRiseRise (('R'<<0)|('i'<<8)|('s'<<16)|('e'<<24))
#define ORC_X86_SiS_SiS_SiS_ (('S'<<0)|('i'<<8)|('S'<<16)|(' '<<24))
#define ORC_X86_UMC_UMC_UMC_ (('U'<<0)|('M'<<8)|('C'<<16)|(' '<<24))
#define ORC_X86_VIA_VIA_VIA_ (('V'<<0)|('I'<<8)|('A'<<16)|(' '<<24))

  switch (vendor_id) {
    case ORC_X86_GenuineIntel:
      orc_x86_cpu_vendor = ORC_X86_CPU_VENDOR_INTEL;
      orc_x86_cpu_intel_detect (orc_x86_cpu_level);
      break;
    case ORC_X86_AuthenticAMD:
    case ORC_X86_HygonGenuine:
      orc_x86_cpu_vendor = ORC_X86_CPU_VENDOR_AMD;
      orc_x86_cpu_amd_detect (orc_x86_cpu_level);
      break;
    default:
      orc_x86_cpu_vendor = ORC_X86_CPU_VENDOR_GENERIC;
      ORC_INFO("unhandled vendor %08x %08x %08x", ebx, edx, vendor_id);
      orc_x86_cpu_generic_detect (orc_x86_cpu_level);
      break;
  }

done:
  if (vendor) *vendor = orc_x86_cpu_vendor;
  if (level) *level = orc_x86_cpu_level;
}

#define XSAVE_ENABLED 1U << 26
#define XSAVE_AVAILABLE 1U << 27
#define XSAVE_BITS (XSAVE_ENABLED | XSAVE_AVAILABLE)

orc_bool
orc_x86_cpu_is_xsave_enabled (void)
{
  orc_uint32 eax, ebx, ecx, edx;
  // https://bugzilla.mozilla.org/show_bug.cgi?id=1854795
  // https://gitlab.freedesktop.org/gstreamer/orc/-/issues/65
  // check for xsave enabled (bit 26) and xsave available (bit 27)
  orc_bool osxsave_enabled;

  orc_x86_cpu_get_cpuid (0x00000001, &eax, &ebx, &ecx, &edx);
  osxsave_enabled = (ecx & XSAVE_BITS) == XSAVE_BITS;
  return osxsave_enabled;
}

orc_uint32
orc_x86_cpu_get_xcr0 (void)
{
  return get_xcr0 ();
}
