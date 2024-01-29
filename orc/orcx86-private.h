#pragma once

#include <orc/orcutils.h>

ORC_BEGIN_DECLS

/* orcprogram-x86.c */
/* FIXME or either rename OrcX86Target to OrcX86Extension or rename the function to be _register_extension */
ORC_INTERNAL void orc_x86_register_extension(OrcTarget *t, OrcX86Target *x86t);

/* orcx86insn.c */
ORC_INTERNAL orc_bool orc_x86_insn_encoding_from_operands (OrcX86InsnEncoding *encoding, int operands, OrcX86InsnPrefix prefix);
ORC_INTERNAL orc_bool orc_x86_insn_validate_no_operands (int operands);
ORC_INTERNAL orc_bool orc_x86_insn_validate_operand1_reg (int reg, OrcX86InsnOperandSize size, int operands);
ORC_INTERNAL orc_bool orc_x86_insn_validate_operand1_imm (int operands);
ORC_INTERNAL orc_bool orc_x86_insn_validate_operand1_mem (int reg, int operands);
ORC_INTERNAL orc_bool orc_x86_insn_validate_operand2_reg (int reg, OrcX86InsnOperandSize size, int operands);
ORC_INTERNAL orc_bool orc_x86_insn_validate_operand2_imm (int operands);
ORC_INTERNAL orc_bool orc_x86_insn_validate_operand2_mem (int reg, int operands);
ORC_INTERNAL orc_bool orc_x86_insn_validate_operand3_imm (int operands);
ORC_INTERNAL OrcX86InsnOperandSize orc_x86_insn_size_to_operand_size (int size);
ORC_INTERNAL void orc_x86_insn_operand_set (OrcX86InsnOperand * op, OrcX86InsnOperandType type, OrcX86InsnOperandSize size, int reg);
ORC_INTERNAL void orc_x86_emit_align (OrcCompiler *p, int align_shift);
ORC_INTERNAL void orc_x86_emit_label (OrcCompiler *p, int label);
ORC_INTERNAL void orc_x86_insn_need_rex (OrcCompiler *c, OrcX86Insn *xinsn);

ORC_END_DECLS
