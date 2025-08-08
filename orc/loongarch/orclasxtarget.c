/*
  Copyright (c) 2025 Loongson Technology Corporation Limited

  Author: jinbo, jinbo@loongson.cn
  Author: hecai yuan, yuanhecai@loongson.cn

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
#include <orc/orcinternal.h>
#include <orc/loongarch/orcloongarch-internal.h>
#include <orc/loongarch/orclasx-internal.h>
#include <orc/loongarch/orcloongarch.h>

static OrcTarget orc_lasx_target = {
  "lasx",
#ifdef HAVE_LOONGARCH
  TRUE,
#else
  FALSE,
#endif
  ORC_VEC_REG_BASE,
  orc_loongarch_target_get_default_flags,
  orc_lasx_compiler_init,
  orc_lasx_compiler_assemble,
  {{0}},
  0,
  NULL,
  NULL,
  NULL,
  orc_loongarch_flush_cache,
};

OrcTarget *
orc_lasx_target_init (void)
{
#ifdef HAVE_LOONGARCH
  if ((orc_loongarch_get_cpu_flags () & ORC_TARGET_LOONGARCH_LASX)) {
    ORC_INFO ("This LOONGARCH CPU supports LASX");
  } else {
    orc_lasx_target.executable = FALSE;
  }
#endif

  orc_target_register (&orc_lasx_target);
  return &orc_lasx_target;
}
