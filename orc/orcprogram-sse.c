
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>

#include <orc/orcprogram.h>
#include <orc/x86.h>
#include <orc/orcutils.h>
#include <orc/orcdebug.h>

#define SIZE 65536


void sse_emit_loop (OrcCompiler *compiler);

void orc_compiler_sse_init (OrcCompiler *compiler);
void orc_compiler_sse_assemble (OrcCompiler *compiler);
void orc_compiler_sse_register_rules (OrcTarget *target);


void orc_compiler_rewrite_vars (OrcCompiler *compiler);
void orc_compiler_dump (OrcCompiler *compiler);

static OrcTarget sse_target = {
  "sse",
#if defined(HAVE_I386) || defined(HAVE_AMD64)
  TRUE,
#else
  FALSE,
#endif
  ORC_VEC_REG_BASE,
  orc_compiler_sse_init,
  orc_compiler_sse_assemble
};


void
orc_sse_init (void)
{
  orc_target_register (&sse_target);

  orc_compiler_sse_register_rules (&sse_target);
}

void
orc_compiler_sse_init (OrcCompiler *compiler)
{
  int i;

  if (x86_64) {
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

  compiler->tmpreg = X86_XMM0;
  compiler->valid_regs[compiler->tmpreg] = 0;

  switch (orc_program_get_max_var_size (compiler->program)) {
    case 1:
      compiler->loop_shift = 4;
      break;
    case 2:
      compiler->loop_shift = 3;
      break;
    case 4:
      compiler->loop_shift = 2;
      break;
    default:
      ORC_ERROR("unhandled max var size %d",
          orc_program_get_max_var_size (compiler->program));
      break;
  }

  //compiler->long_jumps = TRUE;
}

void
sse_load_constants (OrcCompiler *compiler)
{
  int i;
  for(i=0;i<ORC_N_VARIABLES;i++){
    if (compiler->vars[i].name == NULL) continue;
    switch (compiler->vars[i].vartype) {
      case ORC_VAR_TYPE_CONST:
        if (compiler->vars[i].size == 1) {
          sse_emit_loadib (compiler, compiler->vars[i].alloc,
              (int)compiler->vars[i].value);
        } else if (compiler->vars[i].size == 2) {
          sse_emit_loadiw (compiler, compiler->vars[i].alloc,
              (int)compiler->vars[i].value);
        } else if (compiler->vars[i].size == 4) {
          sse_emit_loadil (compiler, compiler->vars[i].alloc,
              (int)compiler->vars[i].value);
        } else {
          ORC_PROGRAM_ERROR(compiler, "unimplemented");
        }
        break;
      case ORC_VAR_TYPE_PARAM:
        if (compiler->vars[i].size == 1) {
          sse_emit_loadpb (compiler, compiler->vars[i].alloc, i);
        } else if (compiler->vars[i].size == 2) {
          sse_emit_loadpw (compiler, compiler->vars[i].alloc, i);
        } else if (compiler->vars[i].size == 4) {
          sse_emit_loadpl (compiler, compiler->vars[i].alloc, i);
        } else {
          ORC_PROGRAM_ERROR(compiler, "unimplemented");
        }
        break;
      case ORC_VAR_TYPE_SRC:
      case ORC_VAR_TYPE_DEST:
        if (compiler->vars[i].ptr_register) {
          x86_emit_mov_memoffset_reg (compiler, x86_ptr_size,
              (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]), x86_exec_ptr,
              compiler->vars[i].ptr_register);
        } else {
          ORC_PROGRAM_ERROR(compiler,"unimplemented");
        }
        break;
      default:
        break;
    }
  }
}

void
sse_emit_load_src (OrcCompiler *compiler, OrcVariable *var)
{
  int ptr_reg;
  if (var->ptr_register == 0) {
    int i;
    i = var - compiler->vars;
    x86_emit_mov_memoffset_reg (compiler, x86_ptr_size,
        (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[i]),
        x86_exec_ptr, X86_ECX);
    ptr_reg = X86_ECX;
  } else {
    ptr_reg = var->ptr_register;
  }
  switch (var->size << compiler->loop_shift) {
    case 1:
      x86_emit_mov_memoffset_reg (compiler, 1, 0, ptr_reg, X86_ECX);
      x86_emit_mov_reg_sse (compiler, X86_ECX, var->alloc);
      break;
    case 2:
      x86_emit_mov_memoffset_reg (compiler, 2, 0, ptr_reg, X86_ECX);
      x86_emit_mov_reg_sse (compiler, X86_ECX, var->alloc);
      break;
    case 4:
      x86_emit_mov_memoffset_sse (compiler, 4, 0, ptr_reg, var->alloc,
          var->is_aligned);
      break;
    case 8:
      x86_emit_mov_memoffset_sse (compiler, 8, 0, ptr_reg, var->alloc,
          var->is_aligned);
      break;
    case 16:
      x86_emit_mov_memoffset_sse (compiler, 16, 0, ptr_reg, var->alloc,
          var->is_aligned);
      break;
    default:
      ORC_PROGRAM_ERROR(compiler,"bad load size %d",
          var->size << compiler->loop_shift);
      break;
  }
}

void
sse_emit_store_dest (OrcCompiler *compiler, OrcVariable *var)
{
  int ptr_reg;
  if (var->ptr_register == 0) {
    x86_emit_mov_memoffset_reg (compiler, x86_ptr_size,
        var->ptr_offset, x86_exec_ptr, X86_ECX);
    ptr_reg = X86_ECX;
  } else {
    ptr_reg = var->ptr_register;
  }
  switch (var->size << compiler->loop_shift) {
    case 1:
      /* FIXME we might be using ecx twice here */
      if (ptr_reg == X86_ECX) {
        ORC_PROGRAM_ERROR(compiler,"unimplemented");
      }
      x86_emit_mov_sse_reg (compiler, var->alloc, X86_ECX);
      x86_emit_mov_reg_memoffset (compiler, 1, X86_ECX, 0, ptr_reg);
      break;
    case 2:
      /* FIXME we might be using ecx twice here */
      if (ptr_reg == X86_ECX) {
        ORC_PROGRAM_ERROR(compiler,"unimplemented");
      }
      x86_emit_mov_sse_reg (compiler, var->alloc, X86_ECX);
      x86_emit_mov_reg_memoffset (compiler, 2, X86_ECX, 0, ptr_reg);
      break;
    case 4:
      x86_emit_mov_sse_memoffset (compiler, 4, var->alloc, 0, ptr_reg,
          var->is_aligned, var->is_uncached);
      break;
    case 8:
      x86_emit_mov_sse_memoffset (compiler, 8, var->alloc, 0, ptr_reg,
          var->is_aligned, var->is_uncached);
      break;
    case 16:
      x86_emit_mov_sse_memoffset (compiler, 16, var->alloc, 0, ptr_reg,
          var->is_aligned, var->is_uncached);
      break;
    default:
      ORC_PROGRAM_ERROR(compiler,"bad size");
      break;
  }
}

static int
get_shift (int size)
{
  switch (size) {
    case 1:
      return 0;
    case 2:
      return 1;
    case 4:
      return 2;
    default:
      ORC_ERROR("bad size %d", size);
  }
  return -1;
}
void
orc_compiler_sse_assemble (OrcCompiler *compiler)
{
  int dest_var;
  int dest_shift;

  dest_var = orc_compiler_get_dest (compiler);
  dest_shift = get_shift (compiler->vars[dest_var].size);

  compiler->vars[dest_var].is_aligned = FALSE;

  x86_emit_prologue (compiler);

  if (compiler->loop_shift > 0) {
    x86_emit_mov_imm_reg (compiler, 4, 16, X86_EAX);
    x86_emit_sub_memoffset_reg (compiler, 4,
        (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[dest_var]),
        x86_exec_ptr, X86_EAX);
    x86_emit_and_imm_reg (compiler, 4, 15, X86_EAX);
    x86_emit_sar_imm_reg (compiler, 4, dest_shift, X86_EAX);

    x86_emit_cmp_reg_memoffset (compiler, 4, X86_EAX,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,n), x86_exec_ptr);

    x86_emit_jle (compiler, 6);

    x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1), x86_exec_ptr);
    
    x86_emit_mov_memoffset_reg (compiler, 4,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,n), x86_exec_ptr, X86_ECX);
    x86_emit_sub_reg_reg (compiler, 4, X86_EAX, X86_ECX);

    x86_emit_mov_reg_reg (compiler, 4, X86_ECX, X86_EAX);

    x86_emit_sar_imm_reg (compiler, 4, compiler->loop_shift, X86_ECX);
    x86_emit_mov_reg_memoffset (compiler, 4, X86_ECX,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2), x86_exec_ptr);

    x86_emit_and_imm_reg (compiler, 4, (1<<compiler->loop_shift)-1, X86_EAX);
    x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter3), x86_exec_ptr);

    x86_emit_jmp (compiler, 7);
    x86_emit_label (compiler, 6);

    x86_emit_mov_memoffset_reg (compiler, 4,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,n), x86_exec_ptr, X86_EAX);
    x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1), x86_exec_ptr);
    x86_emit_mov_imm_reg (compiler, 4, 0, X86_EAX);
    x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2), x86_exec_ptr);
    x86_emit_mov_reg_memoffset (compiler, 4, X86_EAX,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter3), x86_exec_ptr);

    x86_emit_label (compiler, 7);
  } else {
    x86_emit_mov_memoffset_reg (compiler, 4,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,n), x86_exec_ptr, X86_ECX);
    x86_emit_mov_reg_memoffset (compiler, 4, X86_ECX,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2), x86_exec_ptr);
  }

  sse_load_constants (compiler);

  if (compiler->loop_shift > 0) {
    int save_loop_shift;

    x86_emit_cmp_imm_memoffset (compiler, 4, 0,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1), x86_exec_ptr);
    x86_emit_je (compiler, 1);

    save_loop_shift = compiler->loop_shift;
    compiler->loop_shift = 0;

    x86_emit_label (compiler, 0);
    sse_emit_loop (compiler);
    x86_emit_dec_memoffset (compiler, 4,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter1),
        x86_exec_ptr);
    x86_emit_jne (compiler, 0);

    compiler->loop_shift = save_loop_shift;
    compiler->vars[dest_var].is_aligned = TRUE;
  }

  x86_emit_label (compiler, 1);

  x86_emit_cmp_imm_memoffset (compiler, 4, 0,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2), x86_exec_ptr);
  x86_emit_je (compiler, 3);

  x86_emit_align (compiler);
  x86_emit_label (compiler, 2);
  sse_emit_loop (compiler);
  x86_emit_dec_memoffset (compiler, 4,
      (int)ORC_STRUCT_OFFSET(OrcExecutor,counter2),
      x86_exec_ptr);
  x86_emit_jne (compiler, 2);
  x86_emit_label (compiler, 3);

  if (compiler->loop_shift > 0) {
    int save_loop_shift;

    compiler->vars[dest_var].is_aligned = FALSE;
    x86_emit_cmp_imm_memoffset (compiler, 4, 0,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter3), x86_exec_ptr);
    x86_emit_je (compiler, 5);

    save_loop_shift = compiler->loop_shift;
    compiler->loop_shift = 0;

    x86_emit_label (compiler, 4);
    sse_emit_loop (compiler);
    x86_emit_dec_memoffset (compiler, 4,
        (int)ORC_STRUCT_OFFSET(OrcExecutor,counter3),
        x86_exec_ptr);
    x86_emit_jne (compiler, 4);

    x86_emit_label (compiler, 5);

    compiler->loop_shift = save_loop_shift;
  }

  x86_emit_epilogue (compiler);

  x86_do_fixups (compiler);
}

void
sse_emit_loop (OrcCompiler *compiler)
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

    ORC_ASM_CODE(compiler,"# %d: %s\n", j, insn->opcode->name);

#if 0
    /* set up args */
    for(k=0;k<opcode->n_src + opcode->n_dest;k++){
      args[k] = compiler->vars + insn->args[k];
      ORC_ASM_CODE(compiler," %d", args[k]->alloc);
      if (args[k]->is_chained) {
        ORC_ASM_CODE(compiler," (chained)");
      }
    }
    ORC_ASM_CODE(compiler,"\n");
#endif

    for(k=0;k<ORC_STATIC_OPCODE_N_SRC;k++){
      OrcVariable *var = compiler->vars + insn->src_args[k];

      if (opcode->src_size[k] == 0) continue;

      switch (var->vartype) {
        case ORC_VAR_TYPE_SRC:
        case ORC_VAR_TYPE_DEST:
          sse_emit_load_src (compiler, var);
          break;
        case ORC_VAR_TYPE_CONST:
          break;
        case ORC_VAR_TYPE_PARAM:
          break;
        case ORC_VAR_TYPE_TEMP:
          break;
        default:
          break;
      }
    }

    rule = insn->rule;
    if (rule && rule->emit) {
      if (compiler->vars[insn->dest_args[0]].alloc !=
          compiler->vars[insn->src_args[0]].alloc) {
        x86_emit_mov_sse_reg_reg (compiler,
            compiler->vars[insn->src_args[0]].alloc,
            compiler->vars[insn->dest_args[0]].alloc);
      }
      rule->emit (compiler, rule->emit_user, insn);
    } else {
      ORC_PROGRAM_ERROR(compiler,"No rule for: %s", opcode->name);
    }

    for(k=0;k<ORC_STATIC_OPCODE_N_DEST;k++){
      OrcVariable *var = compiler->vars + insn->dest_args[k];

      if (opcode->dest_size[k] == 0) continue;

      switch (var->vartype) {
        case ORC_VAR_TYPE_DEST:
          sse_emit_store_dest (compiler, var);
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
        x86_emit_add_imm_reg (compiler, x86_ptr_size,
            compiler->vars[k].size << compiler->loop_shift,
            compiler->vars[k].ptr_register);
      } else {
        x86_emit_add_imm_memoffset (compiler, x86_ptr_size,
            compiler->vars[k].size << compiler->loop_shift,
            (int)ORC_STRUCT_OFFSET(OrcExecutor, arrays[k]),
            x86_exec_ptr);
      }
    }
  }
}

