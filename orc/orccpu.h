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

#ifndef _ORC_CPU_H_
#define _ORC_CPU_H_

#include <orc/orcutils.h>

ORC_BEGIN_DECLS

#ifdef ORC_ENABLE_UNSTABLE_API

typedef enum {
  ORC_CPU_FLAG_CMOV = (1<<0),
  ORC_CPU_FLAG_MMX = (1<<1),
  ORC_CPU_FLAG_SSE = (1<<2),
  ORC_CPU_FLAG_MMXEXT = (1<<3),
  ORC_CPU_FLAG_SSE2 = (1<<4),
  ORC_CPU_FLAG_3DNOW = (1<<5),
  ORC_CPU_FLAG_3DNOWEXT = (1<<6),
  ORC_CPU_FLAG_SSE3 = (1<<7),
  ORC_CPU_FLAG_ALTIVEC = (1<<8),
  ORC_CPU_FLAG_EDSP = (1<<9),
  ORC_CPU_FLAG_ARM6 = (1<<10),
  ORC_CPU_FLAG_VFP = (1<<11),
  ORC_CPU_FLAG_SSSE3 = (1<<12)
} OrcCpuFlag;

unsigned int orc_sse_get_cpu_flags (void);
unsigned int orc_mmx_get_cpu_flags (void);

void _orc_cpu_init (void);

ORC_EXPORT unsigned long orc_cpu_flags;

#endif

ORC_END_DECLS

#endif

