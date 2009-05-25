
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>

#include <orc/orc.h>
#include <orc/orcdebug.h>
#include <orc/x86.h>

#define SIZE 65536

void mmx_emit_loop (OrcCompiler *compiler);

void orc_compiler_mmx_init (OrcCompiler *compiler);
unsigned int orc_compiler_mmx_get_default_flags (void);
void orc_compiler_mmx_assemble (OrcCompiler *compiler);

void orc_compiler_mmx_register_rules (OrcTarget *target);



void orc_compiler_rewrite_vars (OrcCompiler *compiler);
void orc_compiler_dump (OrcCompiler *compiler);

static OrcTarget mmx_target = {
  "mmx",
#if defined(HAVE_I386) || defined(HAVE_AMD64)
  TRUE,
#else
  FALSE,
#endif
  ORC_VEC_REG_BASE,
  orc_compiler_mmx_get_default_flags,
  orc_compiler_mmx_init,
  orc_compiler_mmx_assemble
};

void
orc_mmx_init (void)
{
  orc_target_register (&mmx_target);

  orc_compiler_mmx_register_rules (&mmx_target);
}

unsigned int
orc_compiler_mmx_get_default_flags (void)
{
  return 0;
}

void
orc_compiler_mmx_init (OrcCompiler *compiler)
{
  int i;

  if (compiler->is_64bit) {
    for(i=ORC_GP_REG_BASE;i<ORC_GP_REG_BASE+16;i++){
      compiler->valid_regs[i] = 1;
    }
    compiler->valid_regs[X86_ECX] = 0;
    compiler->valid_regs[X86_EDI] = 0;
    compiler->valid_regs[X86_ESP] = 0;
    for(i=X86_XMM0;i<X86_XMM0+16;i++){
      compiler->valid_regs[i] = 1;
    }
    compiler->save_regs[X86_EBX] = 1;
    compiler->save_regs[X86_EBP] = 1;
    compiler->save_regs[X86_R12] = 1;
    compiler->save_regs[X86_R13] = 1;
    compiler->save_regs[X86_R14] = 1;
    compiler->save_regs[X86_R15] = 1;
  } else {
    for(i=ORC_GP_REG_BASE;i<ORC_GP_REG_BASE+8;i++){
      compiler->valid_regs[i] = 1;
    }
    compiler->valid_regs[X86_ECX] = 0;
    compiler->valid_regs[X86_ESP] = 0;
    compiler->valid_regs[X86_EBP] = 0;
    for(i=X86_XMM0;i<X86_XMM0+8;i++){
      compiler->valid_regs[i] = 1;
    }
    compiler->save_regs[X86_EBX] = 1;
    compiler->save_regs[X86_EDI] = 1;
    compiler->save_regs[X86_EBP] = 1;
  }
  for(i=X86_MM0;i<X86_MM0+8;i++){
    compiler->valid_regs[i] = 1;
  }
  for(i=0;i<128;i++){
    compiler->alloc_regs[i] = 0;
    compiler->used_regs[i] = 0;
  }

  compiler->loop_shift = 2;
}

void
mmx_load_constants (OrcCompiler *compiler)
{
  int i;
  for(i=0;i<ORC_N_VARIABLES;i++){
    if (compiler->vars[i].name == NULL) continue;
    switch (compiler->vars[i].vartype) {
      case ORC_VAR_TYPE_CONST:
        orc_mmx_emit_loadiw (compiler, compiler->vars[i].alloc,
            (int)compiler->vars[i].value);
        break;
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        if (compiler->vars[i].ptr_register) {
          orc_x86_emit_mov_memoffset_reg (compiler, compiler->is_64bit ? 8 : 4,
              (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]), compiler->exec_reg,
              compiler->vars[i].ptr_register);
        } else {
          ORC_COMPILER_ERROR(compiler, "unimplemented");
        }
        break;
      default:
        break;
    }
  }
}

void
orc_mmx_emit_load_src (OrcCompiler *compiler, OrcVariable *var)
{
  int ptr_reg;
  if (var->ptr_register == 0) {
    orc_x86_emit_mov_memoffset_reg (compiler, compiler->is_64bit ? 8 : 4,
        var->ptr_offset, compiler->exec_reg, X86_ECX);
    ptr_reg = X86_ECX;
  } else {
    ptr_reg = var->ptr_register;
  }
  switch (compiler->loop_shift) {
    case 0:
      orc_x86_emit_mov_memoffset_reg (compiler, 2, 0, ptr_reg, X86_ECX);
      orc_x86_emit_mov_reg_mmx (compiler, X86_ECX, var->alloc);
      break;
    case 1:
      orc_x86_emit_mov_memoffset_mmx (compiler, 4, 0, ptr_reg, var->alloc);
      break;
    case 2:
      orc_x86_emit_mov_memoffset_mmx (compiler, 8, 0, ptr_reg, var->alloc);
      break;
    default:
      ORC_COMPILER_ERROR(compiler, "bad size");
  }
}

void
mmx_emit_store_dest (OrcCompiler *compiler, OrcVariable *var)
{
  int ptr_reg;
  if (var->ptr_register == 0) {
    orc_x86_emit_mov_memoffset_reg (compiler, compiler->is_64bit ? 8 : 4,
        var->ptr_offset, compiler->exec_reg, X86_ECX);
    ptr_reg = X86_ECX;
  } else {
    ptr_reg = var->ptr_register;
  }
  switch (compiler->loop_shift) {
    case 0:
      /* FIXME we might be using ecx twice here */
      if (ptr_reg == X86_ECX) {
        ORC_COMPILER_ERROR(compiler, "unimplemented");
      }
      orc_x86_emit_mov_mmx_reg (compiler, var->alloc, X86_ECX);
      orc_x86_emit_mov_reg_memoffset (compiler, 2, X86_ECX, 0, ptr_reg);
      break;
    case 1:
      orc_x86_emit_mov_mmx_memoffset (compiler, 4, var->alloc, 0, ptr_reg);
      break;
    case 2:
      orc_x86_emit_mov_mmx_memoffset (compiler, 8, var->alloc, 0, ptr_reg);
      break;
    default:
      ORC_COMPILER_ERROR(compiler, "unimplemented");
  }
}

void
orc_compiler_mmx_assemble (OrcCompiler *compiler)
{
  orc_x86_emit_prologue (compiler);

  orc_x86_emit_mov_memoffset_reg (compiler, 4, (int)ORC_STRUCT_OFFSET(OrcExecutor,n),
      compiler->exec_reg, X86_ECX);
  orc_x86_emit_sar_imm_reg (compiler, 4, compiler->loop_shift, X86_ECX);
  orc_x86_emit_mov_reg_memoffset (compiler, 4, X86_ECX,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2), compiler->exec_reg);

  orc_x86_emit_mov_memoffset_reg (compiler, 4,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,n), compiler->exec_reg, X86_ECX);
  orc_x86_emit_and_imm_reg (compiler, 4, (1<<compiler->loop_shift)-1, X86_ECX);
  orc_x86_emit_mov_reg_memoffset (compiler, 4, X86_ECX,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1), compiler->exec_reg);

  mmx_load_constants (compiler);

  if (compiler->loop_shift > 0) {
    int save_loop_shift;

    orc_x86_emit_cmp_imm_memoffset (compiler, 4, 0,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1), compiler->exec_reg);
    orc_x86_emit_je (compiler, 1);

    save_loop_shift = compiler->loop_shift;
    compiler->loop_shift = 0;

    orc_x86_emit_label (compiler, 0);
    mmx_emit_loop (compiler);
    orc_x86_emit_dec_memoffset (compiler, 4,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1),
        compiler->exec_reg);
    orc_x86_emit_jne (compiler, 0);

    compiler->loop_shift = save_loop_shift;
  }

  orc_x86_emit_label (compiler, 1);

  orc_x86_emit_cmp_imm_memoffset (compiler, 4, 0,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2), compiler->exec_reg);
  orc_x86_emit_je (compiler, 3);

  orc_x86_emit_label (compiler, 2);
  mmx_emit_loop (compiler);
  orc_x86_emit_dec_memoffset (compiler, 4,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2),
      compiler->exec_reg);
  orc_x86_emit_jne (compiler, 2);
  orc_x86_emit_label (compiler, 3);

  orc_x86_emit_emms (compiler);
  orc_x86_emit_epilogue (compiler);

  x86_do_fixups (compiler);
}

void
mmx_emit_loop (OrcCompiler *compiler)
{
  int j;
  int k;
  OrcInstruction *insn;
  OrcStaticOpcode *opcode;
  //OrcVariable *args[10];
  OrcRule *rule;

  for(j=0;j<compiler->n_insns;j++){
    insn = compiler->insns + j;
    opcode = insn->opcode;

    orc_compiler_append_code(compiler,"# %d: %s\n", j, insn->opcode->name);

#if 0
    /* set up args */
    for(k=0;k<opcode->n_src + opcode->n_dest;k++){
      args[k] = compiler->vars + insn->args[k];
      orc_compiler_append_code(compiler," %d", args[k]->alloc);
      if (args[k]->is_chained) {
        orc_compiler_append_code(compiler," (chained)");
      }
    }
    orc_compiler_append_code(compiler,"\n");
#endif

    for(k=0;k<ORC_STATIC_OPCODE_N_SRC;k++){
      OrcVariable *var = compiler->vars + insn->src_args[k];

      if (opcode->src_size[k] == 0) continue;

      switch (var->vartype) {
        case ORC_VAR_TYPE_SRC:
          orc_mmx_emit_load_src (compiler, var);
          break;
        case ORC_VAR_TYPE_CONST:
          break;
        case ORC_VAR_TYPE_TEMP:
          break;
        default:
          break;
      }
    }

    rule = insn->rule;
    if (rule) {
      if (compiler->vars[insn->dest_args[0]].alloc !=
          compiler->vars[insn->src_args[0]].alloc) {
        orc_x86_emit_mov_mmx_reg_reg (compiler,
            compiler->vars[insn->src_args[0]].alloc,
            compiler->vars[insn->dest_args[0]].alloc);
      }
      rule->emit (compiler, rule->emit_user, insn);
    } else {
      orc_compiler_append_code(compiler,"No rule for: %s\n", opcode->name);
    }

    for(k=0;k<ORC_STATIC_OPCODE_N_DEST;k++){
      OrcVariable *var = compiler->vars + insn->dest_args[k];

      if (opcode->dest_size[k] == 0) continue;

      switch (var->vartype) {
        case ORC_VAR_TYPE_DEST:
          mmx_emit_store_dest (compiler, var);
          break;
        case ORC_VAR_TYPE_TEMP:
          break;
        default:
          break;
      }
    }
  }

  for(k=0;k<ORC_N_VARIABLES;k++){
    if (compiler->vars[k].name == NULL) continue;
    if (compiler->vars[k].vartype == ORC_VAR_TYPE_SRC ||
        compiler->vars[k].vartype == ORC_VAR_TYPE_DEST) {
      if (compiler->vars[k].ptr_register) {
        orc_x86_emit_add_imm_reg (compiler, compiler->is_64bit ? 8 : 4,
            compiler->vars[k].size << compiler->loop_shift,
            compiler->vars[k].ptr_register);
      } else {
        orc_x86_emit_add_imm_memoffset (compiler, compiler->is_64bit ? 8 : 4,
            compiler->vars[k].size << compiler->loop_shift,
            (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[k]),
            compiler->exec_reg);
      }
    }
  }
}

