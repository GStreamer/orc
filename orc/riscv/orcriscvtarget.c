/*
  Copyright © 2024-2025 Samsung Electronics

  Author: Maksymilian Knust, m.knust@samsung.com
  Author: Filip Wasil, f.wasil@samsung.com

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <orc/orcdebug.h>
#include <orc/orctarget.h>
#include <orc/orcinternal.h>
#include <orc/orcutils-private.h>
#include <orc/orcutils.h>
#include <orc/orccode.h>
#include <orc/riscv/orcriscv-internal.h>

#include <unistd.h>

#if defined(HAVE_LINUX_RVV) || defined(HAVE_ELF_AUX_INFO)
#include <sys/auxv.h>
#endif

#ifdef HAVE_LINUX_RVV
#include <sys/hwprobe.h>
#include <asm/hwcap.h>
#include <asm/hwprobe.h>
#include <asm/unistd.h>
#elif defined(__linux__)
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#endif

#if defined(HAVE_RISCV)
#if defined(HAVE_ELF_AUX_INFO)
static orc_uint32
orc_check_riscv_elf_aux_info (void)
{
  unsigned long flags = 0;
  unsigned long hwcap = 0;
#ifdef HWCAP2_ISA_ZVBB
  unsigned long hwcap2 = 0;
#endif

  elf_aux_info(AT_HWCAP, &hwcap, sizeof(hwcap));
#ifdef HWCAP2_ISA_ZVBB
  elf_aux_info(AT_HWCAP2, &hwcap2, sizeof(hwcap2));
#endif

  if (hwcap & HWCAP_ISA_C)
    flags |= ORC_TARGET_RISCV_C;
#ifdef HWCAP_ISA_V
  if (hwcap & HWCAP_ISA_V)
    flags |= ORC_TARGET_RISCV_V;
#endif
#ifdef HWCAP2_ISA_ZVBB
  if (hwcap2 & HWCAP2_ISA_ZVBB)
    flags |= ORC_TARGET_RISCV_ZVBB;
#endif

  return flags;
}
#elif defined(HAVE_LINUX_RVV)
static orc_uint32
orc_check_riscv_hwprobe_getauxval (void)
{
  unsigned long flags = 0;
  struct riscv_hwprobe pair[] = {
    { RISCV_HWPROBE_KEY_IMA_EXT_0, 0},
  };

  if (__riscv_hwprobe(pair, 1, 0, NULL, 0) < 0)
    return 0;

  if (pair[0].value & RISCV_HWPROBE_IMA_C)
    flags |= ORC_TARGET_RISCV_C;
  if (getauxval (AT_HWCAP) & COMPAT_HWCAP_ISA_V) {
    if (pair[0].value & RISCV_HWPROBE_IMA_V)
      flags |= ORC_TARGET_RISCV_V;
  }
#ifdef RISCV_HWPROBE_EXT_ZVKB
  if (pair[0].value & RISCV_HWPROBE_EXT_ZVKB)
    flags |= ORC_TARGET_RISCV_ZVKB;
#endif
#ifdef RISCV_HWPROBE_EXT_ZVBB
  if (pair[0].value & RISCV_HWPROBE_EXT_ZVBB)
    flags |= ORC_TARGET_RISCV_ZVBB;
#endif

  /* ORC_TARGET_RISCV_ZVKN */
  /* ORC_TARGET_RISCV_ZVKS */

  return flags;
}
#elif defined(__linux__)
static int
orc_riscv_target_detect_extension (const char *exts, const char *ext)
{
  while (isspace (*exts))
    exts++;

  ORC_ASSERT (strncmp (exts, "rv", 2) == 0);

  const int n = strlen (ext);
  char *sep = strchr (exts, '_');

  if (n == 1) {
    char *found = strstr (exts + 2, ext);
    return found && (found < sep || sep == NULL);
  }

  for (; sep; sep = strchr (sep + 1, '_')) {
    if (strncmp (sep + 1, ext, n) && !isalnum (exts[n]))
      return TRUE;
  }

  return FALSE;
}

static orc_uint32
orc_check_riscv_proc_cpuinfo (void)
{
  unsigned long flags = 0;
  char *cpuinfo;
  char *cpuinfo_line;

  cpuinfo = get_proc_cpuinfo ();
  if (cpuinfo == NULL) {
    ORC_DEBUG ("Failed to read /proc/cpuinfo");
    return 0;
  }

  cpuinfo_line = get_tag_value (cpuinfo, "isa");
  if (cpuinfo_line) {
    if (orc_riscv_target_detect_extension (cpuinfo_line, "c"))
      flags |= ORC_TARGET_RISCV_C;
    if (orc_riscv_target_detect_extension (cpuinfo_line, "v"))
      flags |= ORC_TARGET_RISCV_V;
    if (orc_riscv_target_detect_extension (cpuinfo_line, "zvkb"))
      flags |= ORC_TARGET_RISCV_ZVKB;
    if (orc_riscv_target_detect_extension (cpuinfo_line, "zvbb"))
      flags |= ORC_TARGET_RISCV_ZVBB;
    if (orc_riscv_target_detect_extension (cpuinfo_line, "zvkn"))
      flags |= ORC_TARGET_RISCV_ZVKN;
    if (orc_riscv_target_detect_extension (cpuinfo_line, "zvks"))
      flags |= ORC_TARGET_RISCV_ZVKS;

    free (cpuinfo_line);
  }

  free (cpuinfo);

  return flags;
}
#endif
#endif // HAVE_RISCV

static orc_uint32
orc_riscv_target_get_cpu_flags (void)
{
  orc_uint32 flags = 0;

#if defined(__riscv_xlen) && __riscv_xlen == 64
  flags |= ORC_TARGET_RISCV_64BIT;
#endif

#if defined(HAVE_RISCV)
#if defined(HAVE_ELF_AUX_INFO)
  flags |= orc_check_riscv_elf_aux_info ();
#elif defined(HAVE_LINUX_RVV)
  flags |= orc_check_riscv_hwprobe_getauxval ();
#elif defined(__linux__)
  flags |= orc_check_riscv_proc_cpuinfo ();
#endif
#endif // HAVE_RISCV

  return flags;
}

static void
orc_riscv_target_flush_cache (OrcCode *code)
{
#if defined(HAVE_RISCV) && (defined(__linux__) || defined(__OpenBSD__))
#if __has_builtin(__builtin___clear_cache)
  __builtin___clear_cache ((char*)code->code, (char*)code->code + code->code_size);
  if ((void *) code->exec != (void *) code->code)
    __builtin___clear_cache ((char*)code->exec, (char*)code->exec + code->code_size);
#else
  __clear_cache (code->code, code->code + code->code_size);
  if ((void *) code->exec != (void *) code->code)
    __clear_cache (code->exec, code->exec + code->code_size);
#endif // __builtin___clear_cache
#else
  ORC_ERROR ("Couldn't flush icache");
  ORC_ASSERT (0);
#endif
}

static orc_uint32
orc_riscv_target_get_default_flags (void)
{
  orc_uint32 flags = 0;

#if defined(__riscv_xlen) && __riscv_xlen == 64
  flags |= ORC_TARGET_RISCV_64BIT;
#endif

  flags |= orc_riscv_target_get_cpu_flags ();

  return flags;
}

static OrcTarget orc_riscv_target = {
  "riscv",
#ifdef HAVE_RISCV
  TRUE,
#else
  FALSE,
#endif
  ORC_VEC_REG_BASE,
  orc_riscv_target_get_default_flags,
  orc_riscv_compiler_init,
  orc_riscv_compiler_assemble,
  {{0}},
  0,
  NULL,
  NULL,
  NULL,
  orc_riscv_target_flush_cache,
};

OrcTarget *
orc_riscv_target_init (void)
{
#ifdef HAVE_RISCV
  if (orc_riscv_target_get_cpu_flags () & ORC_TARGET_RISCV_V) {
    ORC_INFO ("This RISC-V CPU supports RVV");
  }
#endif
  orc_target_register (&orc_riscv_target);
  return &orc_riscv_target;
}
