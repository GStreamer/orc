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

#if defined(__linux__)
#include <sys/auxv.h>
#endif

#ifndef LA_HWCAP_LSX
#define LA_HWCAP_LSX (1u << 4)
#endif
#ifndef LA_HWCAP_LASX
#define LA_HWCAP_LASX (1u << 5)
#endif

const char *
orc_loongarch_reg_name (OrcLoongRegister reg)
{
  static const char *names[] = {
    [ORC_GP_REG_BASE] = "$zero",
    "$ra", "$tp", "$sp", "$a0", "$a1", "$a2", "$a3",
    "$a4", "$a5", "$a6", "$a7", "$t0", "$t1", "$t2", "$t3",
    "$t4", "$t5", "$t6", "$t7", "$t8", "$r21", "$fp", "$s0",
    "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7", "$s8",

    [ORC_VEC_REG_BASE] = "$vr0",
    "$vr1", "$vr2", "$vr3", "$vr4", "$vr5", "$vr6", "$vr7", "$vr8",
    "$vr9", "$vr10", "$vr11", "$vr12", "$vr13", "$vr14", "$vr15", "$vr16",
    "$vr17", "$vr18", "$vr19", "$vr20", "$vr21", "$vr22", "$vr23", "$vr24",
    "$vr25", "$vr26", "$vr27", "$vr28", "$vr29", "$vr30", "$vr31",

    [ORC_VEC_REG_BASE + 32] = "$xr0",
    "$xr1", "$xr2", "$xr3", "$xr4", "$xr5", "$xr6", "$xr7", "$xr8",
    "$xr9", "$xr10", "$xr11", "$xr12", "$xr13", "$xr14", "$xr15", "$xr16",
    "$xr17", "$xr18", "$xr19", "$xr20", "$xr21", "$xr22", "$xr23", "$xr24",
    "$xr25", "$xr26", "$xr27", "$xr28", "$xr29", "$xr30", "$xr31"
  };

  ORC_ASSERT (reg < ARRAY_SIZE (names));
  ORC_ASSERT (names[reg] != NULL);

  return names[reg];
}

orc_uint32
orc_loongarch_get_cpu_flags (void)
{
  orc_uint32 flags = 0;
#if __linux__
  orc_uint32 flag = getauxval(AT_HWCAP);

  if (flag & LA_HWCAP_LSX)
      flags |= ORC_TARGET_LOONGARCH_LSX;

  if (flag & LA_HWCAP_LASX)
      flags |= ORC_TARGET_LOONGARCH_LASX;
#endif
  return flags;
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

void
orc_loongarch_insn_emit32 (OrcCompiler *const c, const orc_uint32 insn)
{
  ORC_WRITE_UINT32_LE (c->codeptr, insn);
  c->codeptr+=4;
}
