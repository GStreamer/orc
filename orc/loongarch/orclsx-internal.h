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

#ifndef _ORC_LSX_INTERNAL_H_
#define _ORC_LSX_INTERNAL_H_

#include <orc/orcutils.h>

ORC_BEGIN_DECLS

/* orclsxcompiler.c */
ORC_INTERNAL void orc_lsx_compiler_init (OrcCompiler *c);
ORC_INTERNAL void orc_lsx_compiler_assemble (OrcCompiler * c);
ORC_INTERNAL void orc_lsx_compiler_emit_prologue (OrcCompiler *c);
ORC_INTERNAL void orc_lsx_compiler_emit_epilogue (OrcCompiler *c);
ORC_INTERNAL void orc_lsx_compiler_compute_loop_shift (OrcCompiler *c);
ORC_INTERNAL void orc_lsx_compiler_load_constants (OrcCompiler *c);
ORC_INTERNAL void orc_lsx_compiler_save_accumulators (OrcCompiler *c);
/* orclsxrules.c */
ORC_INTERNAL void orc_lsx_rules_init (OrcTarget * target);
/* orclsxtarget.c */
ORC_INTERNAL OrcTarget *orc_lsx_target_init (void);

ORC_END_DECLS

#endif /* _ORC_LSX_INTERNAL_H_ */
