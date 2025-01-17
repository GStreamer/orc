
#ifndef _ORC_X86_H_
#define _ORC_X86_H_

#include <orc/orcprogram.h>
#include <orc/orcx86insn.h>

ORC_BEGIN_DECLS

#ifdef ORC_ENABLE_UNSTABLE_API

/* This struct is a naive wrapper of on top of the common
 * code found on MMX, SSE and AVX. This could be
 * abstracted even more, but as an initial step it's fine.
 */
typedef struct _OrcX86Target
{
  /* Same as OrcTarget */
  const char *name;
  unsigned int (*get_default_flags)(void);
  const char * (*get_flag_name)(int shift);
  int (*is_executable)(void);

  /* X86 specific */
  void (*validate_registers)(int *regs, int is_64bit);
  void (*saveable_registers)(int *regs, int is_64bit);
  int (*is_64bit)(int flags);
  int (*use_frame_pointer)(int flags);
  int (*use_long_jumps)(int flags);
  int (*loop_shift)(int max_var_size);
  void (*init_accumulator)(OrcCompiler *c, OrcVariable *var);
  void (*reduce_accumulator)(OrcCompiler *c, int i, OrcVariable *var);
  void (*load_constant)(OrcCompiler *c, int reg, int size, orc_uint64 value);
  void (*load_constant_long)(OrcCompiler *c, int reg, OrcConstant *constant);
  void (*move_register_to_memoffset)(OrcCompiler *compiler, int size, int reg1, int offset, int reg2, int aligned, int uncached);
  void (*move_memoffset_to_register)(OrcCompiler *compiler, int size, int offset, int reg1, int reg2, int is_aligned);
  int (*get_shift)(int size);
  /* These are specific to the implementation. We need to keep private data
   * and proceed accordingly with a generic prologue, epilogue instead of
   * a function pointer for each case
   */
  void (*set_mxcsr)(OrcCompiler *c);
  void (*restore_mxcsr)(OrcCompiler *c);
  void (*clear_emms)(OrcCompiler *c);
  void (*zeroupper)(OrcCompiler *c);
  int register_size;
  int register_start;
  int n_registers;
  int label_step_up;
} OrcX86Target;

enum {
  X86_EAX = ORC_GP_REG_BASE,
  X86_ECX,
  X86_EDX,
  X86_EBX,
  X86_ESP,
  X86_EBP,
  X86_ESI,
  X86_EDI,
  X86_R8,
  X86_R9,
  X86_R10,
  X86_R11,
  X86_R12,
  X86_R13,
  X86_R14,
  X86_R15
};

enum {
  ORC_X86_UNKNOWN,
  ORC_X86_P6,
  ORC_X86_NETBURST,
  ORC_X86_CORE,
  ORC_X86_PENRYN,
  ORC_X86_NEHALEM,
  ORC_X86_BONNELL,
  ORC_X86_WESTMERE,
  ORC_X86_SANDY_BRIDGE,
  ORC_X86_K5,
  ORC_X86_K6,
  ORC_X86_K7,
  ORC_X86_K8,
  ORC_X86_K10
};

ORC_API const char * orc_x86_get_regname(int i);
ORC_API int          orc_x86_get_regnum(int i);
ORC_API const char * orc_x86_get_regname_8(int i);
ORC_API const char * orc_x86_get_regname_16(int i);
ORC_API const char * orc_x86_get_regname_64(int i);
ORC_API const char * orc_x86_get_regname_ptr(OrcCompiler *compiler, int i);
ORC_API const char * orc_x86_get_regname_size(int i, int size);

ORC_API int  orc_x86_assemble_copy_check (OrcCompiler *compiler);

#endif

ORC_END_DECLS


#endif

