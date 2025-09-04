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

#ifndef _ORC_RISCV_INTERNAL_H_
#define _ORC_RISCV_INTERNAL_H_

#include <orc/orcutils.h>
#include <orc/riscv/orcriscv.h>

ORC_BEGIN_DECLS

#ifdef __riscv
#define HAVE_RISCV
#endif

/* orcriscvcompiler.c */
ORC_INTERNAL void orc_riscv_compiler_assemble (OrcCompiler * c);
ORC_INTERNAL void orc_riscv_compiler_init (OrcCompiler * c);
ORC_INTERNAL void orc_riscv_compiler_add_fixup (OrcCompiler *c, int label);
ORC_INTERNAL OrcRiscvSEW orc_riscv_compiler_bytes_to_sew (int bytes);
ORC_INTERNAL OrcRiscvVtype orc_riscv_compiler_compute_vtype (OrcCompiler *c, OrcRiscvSEW element_width, int insn_shift);
ORC_INTERNAL orc_bool orc_riscv_compiler_get_temp_regs (OrcCompiler *c, OrcInstruction *insn, OrcRiscvRegister *result);
ORC_INTERNAL OrcRiscvRegister orc_riscv_compiler_get_constant (OrcCompiler *c, orc_uint64 n);

/* orcriscvrules.c */
ORC_INTERNAL void orc_riscv_rules_init (OrcTarget * target);

/* orcriscvtarget.c */
ORC_INTERNAL OrcTarget *orc_riscv_target_init (void);

ORC_END_DECLS

#endif /* _ORC_RISCV_INTERNAL_H_ */
