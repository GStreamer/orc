#pragma once

#include <orc/orcutils.h>

ORC_BEGIN_DECLS

/* orccpu-x86.c */
typedef enum _OrcX86CPUVendor
{
  ORC_X86_CPU_VENDOR_INTEL,
  ORC_X86_CPU_VENDOR_AMD,
  ORC_X86_CPU_VENDOR_GENERIC = 100,
} OrcX86CPUVendor;

#if defined(HAVE_I386) || defined(HAVE_AMD64)
ORC_INTERNAL orc_uint32 orc_x86_cpu_get_xcr0 (void);
ORC_INTERNAL orc_bool orc_x86_cpu_is_xsave_enabled (void);
ORC_INTERNAL void orc_x86_cpu_detect (orc_uint32 *level, OrcX86CPUVendor *vendor);
ORC_INTERNAL void orc_x86_cpu_get_cpuid (orc_uint32 op, orc_uint32 *a,
    orc_uint32 *b, orc_uint32 *c, orc_uint32 *d);
#endif

/* orcprogram-x86.c */
/* FIXME or either rename OrcX86Target to OrcX86Extension or rename the function to be _register_extension */
ORC_INTERNAL void orc_x86_register_extension(OrcTarget *t, OrcX86Target *x86t);

/* orcx86insn.c */
ORC_INTERNAL orc_bool orc_x86_insn_encoding_from_operands (OrcX86InsnEncoding *encoding, int operands, OrcX86InsnPrefix prefix);
ORC_INTERNAL orc_bool orc_x86_insn_validate_no_operands (int operands);
ORC_INTERNAL orc_bool orc_x86_insn_validate_operand1_reg (int reg, OrcX86InsnOperandSize size, int operands);
ORC_INTERNAL orc_bool orc_x86_insn_validate_operand1_imm (orc_int64 imm, int operands);
ORC_INTERNAL orc_bool orc_x86_insn_validate_operand1_mem (int reg, int operands);
ORC_INTERNAL orc_bool orc_x86_insn_validate_operand2_reg (int reg, OrcX86InsnOperandSize size, int operands);
ORC_INTERNAL orc_bool orc_x86_insn_validate_operand2_imm (orc_int64 imm, int operands);
ORC_INTERNAL orc_bool orc_x86_insn_validate_operand2_mem (int reg, int operands);
ORC_INTERNAL orc_bool orc_x86_insn_validate_operand3_imm (orc_int64 imm, int operands);
ORC_INTERNAL OrcX86InsnOperandSize orc_x86_insn_size_to_operand_size (int size);
ORC_INTERNAL void orc_x86_insn_operand_set (OrcX86InsnOperand * op, OrcX86InsnOperandType type, OrcX86InsnOperandSize size, int reg);
ORC_INTERNAL void orc_x86_emit_align (OrcCompiler *p, int align_shift);
ORC_INTERNAL void orc_x86_emit_label (OrcCompiler *p, int label);

ORC_END_DECLS
