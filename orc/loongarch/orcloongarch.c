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

#include <orc/orcdebug.h>
#include <orc/orctarget.h>
#include <orc/orccompiler.h>
#include <orc/orcinternal.h>
#include <orc/loongarch/orcloongarch.h>

orc_uint32
orc_loongarch_get_cpu_flags (void)
{
  orc_uint32 ret = 0;
  //TODO
  return ret;
}

void
orc_loongarch_flush_cache (OrcCode *code)
{
#if defined(__loongarch64)
#if __has_builtin(__builtin___clear_cache)
  __builtin___clear_cache ((char*)code->code, (char*)code->code + code->code_size);
  if ((void *) code->exec != (void *) code->code)
    __builtin___clear_cache ((char*)code->exec, (char*)code->exec + code->code_size);
#else
  __clear_cache (code->code, code->code + code->code_size);
  if ((void *) code->exec != (void *) code->code)
    __clear_cache (code->exec, code->exec + code->code_size);
#endif
#endif
}

orc_uint32
orc_loongarch_target_get_default_flags (void)
{
  orc_uint32 flags = 0;

  flags |= orc_loongarch_get_cpu_flags ();

#if defined(__loongarch64)
  flags |= ORC_TARGET_LOONGARCH_64BIT;
#endif

  if (orc_compiler_is_debug ()) {
    flags |= ORC_TARGET_LOONGARCH_FRAME_POINTER;
  }

  return flags;
}
