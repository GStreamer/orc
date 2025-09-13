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
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

#if defined(HAVE_RISCV) && defined(__linux__)
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

  do {
    if (strncmp (sep + 1, ext, n) && !isalnum (exts[n]))
      return TRUE;
  } while ((sep = strchr (sep + 1, '_')));

  return FALSE;
}

static orc_uint32
orc_riscv_target_get_cpu_flags (void)
{
  orc_uint32 ret = 0;

#if defined(__riscv_xlen) && __riscv_xlen == 64
  ret |= ORC_TARGET_RISCV_64BIT;
#endif

  char *cpuinfo = get_proc_cpuinfo ();
  if (cpuinfo == NULL) {
    ORC_DEBUG ("Failed to read /proc/cpuinfo");
    return 0;
  }

  char *cpuinfo_line = get_tag_value (cpuinfo, "isa");
  if (cpuinfo_line) {
    if (orc_riscv_target_detect_extension (cpuinfo_line, "c"))
      ret |= ORC_TARGET_RISCV_C;

    if (orc_riscv_target_detect_extension (cpuinfo_line, "v"))
      ret |= ORC_TARGET_RISCV_V;

    if (orc_riscv_target_detect_extension (cpuinfo_line, "zvkb"))
      ret |= ORC_TARGET_RISCV_ZVKB;

    if (orc_riscv_target_detect_extension (cpuinfo_line, "zvbb"))
      ret |= ORC_TARGET_RISCV_ZVBB;

    if (orc_riscv_target_detect_extension (cpuinfo_line, "zvkn"))
      ret |= ORC_TARGET_RISCV_ZVKN;

    if (orc_riscv_target_detect_extension (cpuinfo_line, "zvks"))
      ret |= ORC_TARGET_RISCV_ZVKS;

    free (cpuinfo_line);
  }

  return ret;
}

#endif

static void
orc_riscv_target_flush_cache (OrcCode *code)
{
#if defined(HAVE_RISCV) && defined(__linux__)
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
#if defined(HAVE_RISCV) && defined(__linux__)
  return orc_riscv_target_get_cpu_flags ();
#else
  return ORC_TARGET_RISCV_64BIT | ORC_TARGET_RISCV_V;
#endif
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
