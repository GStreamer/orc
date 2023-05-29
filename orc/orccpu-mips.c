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
#include <orc/orcmips.h>
#include <orc/orcutils.h>
#include <orc/orcdebug.h>
#include <orc/orcutils-private.h>

#include <unistd.h>
#include <fcntl.h>
#include <string.h>

/***** mips *****/

#ifdef __mips__

unsigned long
orc_mips_get_cpu_flags (void)
{
  unsigned long ret = 0;
  char *cpuinfo;
  char *cpuinfo_line;
  char **flags;
  char **f;

  cpuinfo = get_proc_cpuinfo();
  if (cpuinfo == NULL) {
    ORC_DEBUG ("Failed to read /proc/cpuinfo");
    return 0;
  }

  cpuinfo_line = get_tag_value(cpuinfo, "ASEs implemented");
  if (cpuinfo_line == NULL) {
    free (cpuinfo);
    return 0;
  }

  flags = strsplit(cpuinfo_line, ' ');
  for (f = flags; *f; f++) {
    if (strcmp (*f, "dsp2") == 0)
      ret |= ORC_TARGET_MIPS_DSP2;
    free (*f);
  }

  free (flags);
  free (cpuinfo_line);
  free (cpuinfo);

  return ret;
}
#endif
