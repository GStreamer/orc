
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>

#include <orc/orcprogram.h>
#include <orc/orcinternal.h>
#include <orc/orcx86.h>
#include <orc/orcdebug.h>
#include <orc/orcutils.h>
#include <orc/orcx86insn.h>
#include <orc/orcsse.h>


/**
 * SECTION:orcx86
 * @title: x86
 * @short_description: code generation for x86
 */

const char *
orc_x86_get_regname(int i)
{
  static const char *x86_regs[] = {
    "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi",
    "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d" };

  if (i>=ORC_GP_REG_BASE && i<ORC_GP_REG_BASE + 16) return x86_regs[i - ORC_GP_REG_BASE];
  switch (i) {
    case 0:
      return "UNALLOCATED";
    case 1:
      return "direct";
    default:
      return "ERROR";
  }
}

int
orc_x86_get_regnum(int i)
{
  return (i&0xf);
}

const char *
orc_x86_get_regname_8(int i)
{
  static const char *x86_regs[] = { "al", "cl", "dl", "bl",
    "ah", "ch", "dh", "bh" };

  if (i>=ORC_GP_REG_BASE && i<ORC_GP_REG_BASE + 8) return x86_regs[i - ORC_GP_REG_BASE];
  switch (i) {
    case 0:
      return "UNALLOCATED";
    case 1:
      return "direct";
    default:
      return "ERROR";
  }
}

const char *
orc_x86_get_regname_16(int i)
{
  static const char *x86_regs[] = { "ax", "cx", "dx", "bx",
    "sp", "bp", "si", "di" };

  if (i>=ORC_GP_REG_BASE && i<ORC_GP_REG_BASE + 8) return x86_regs[i - ORC_GP_REG_BASE];
  switch (i) {
    case 0:
      return "UNALLOCATED";
    case 1:
      return "direct";
    default:
      return "ERROR";
  }
}

const char *
orc_x86_get_regname_64(int i)
{
  static const char *x86_regs[] = {
    "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
    "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15" };

  if (i>=ORC_GP_REG_BASE && i<ORC_GP_REG_BASE + 16) return x86_regs[i - ORC_GP_REG_BASE];
  switch (i) {
    case 0:
      return "UNALLOCATED";
    case 1:
      return "direct";
    default:
      return "ERROR";
  }
}

const char *
orc_x86_get_regname_ptr(OrcCompiler *compiler, int i)
{
  if (compiler->is_64bit) {
    return orc_x86_get_regname_64 (i);
  } else {
    return orc_x86_get_regname (i);
  }
}

const char *
orc_x86_get_regname_size(int i, int size)
{
  switch (size) {
    case 1:
      return orc_x86_get_regname_8 (i);
    case 2:
      return orc_x86_get_regname_16 (i);
    case 4:
      return orc_x86_get_regname (i);
    case 8:
      return orc_x86_get_regname_64 (i);
  }
  return NULL;
}


/* FIXME move this to the correct file */
/* memcpy implementation based on rep movs */
int
orc_x86_assemble_copy_check (OrcCompiler *compiler)
{
  if (compiler->program->n_insns == 1 &&
      compiler->program->is_2d == FALSE &&
      (strcmp (compiler->program->insns[0].opcode->name, "copyb") == 0 ||
      strcmp (compiler->program->insns[0].opcode->name, "copyw") == 0 ||
      strcmp (compiler->program->insns[0].opcode->name, "copyl") == 0)) {
    return TRUE;
  }

  return FALSE;
}
