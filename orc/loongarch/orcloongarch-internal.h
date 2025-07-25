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

#ifndef _ORC_LOONGARCH_INTERNAL_H_
#define _ORC_LOONGARCH_INTERNAL_H_

#include <orc/orcutils.h>

ORC_BEGIN_DECLS

ORC_INTERNAL void orc_loongarch_compiler_add_label (OrcCompiler *c, int label);
ORC_INTERNAL void orc_loongarch_compiler_do_fixups (OrcCompiler *c);
ORC_INTERNAL void orc_loongarch_compiler_add_fixup (OrcCompiler *c, int label, int type);
ORC_INTERNAL orc_uint32 orc_loongarch_compiler_get_shift (orc_uint32 bytes);
ORC_INTERNAL void orc_loongarch_compiler_add_strides (OrcCompiler *c);
ORC_INTERNAL void orc_loongarch_compiler_emit_loop (OrcCompiler *c, int update);
ORC_INTERNAL void orc_loongarch_compiler_emit_full_loop (OrcCompiler *c);

ORC_END_DECLS

#endif /* _ORC_LOONGARCH_INTERNAL_H_ */
