
#ifndef _ORC_INTERNAL_H_
#define _ORC_INTERNAL_H_

#include <orc/orcutils.h>
#include <orc/orclimits.h>
#include <orc/orcrule.h>
#include <orc/orcconstant.h>
#include <orc/orcinstruction.h>

ORC_BEGIN_DECLS

/* orccompiler.c */
typedef struct _OrcFixup OrcFixup;

/**
 * OrcFixup:
 *
 * The OrcFixup structure has no public members
 */
struct _OrcFixup {
  /*< private >*/
  unsigned char *ptr;
  int type;
  int label;
};

/**
 * OrcCompiler:
 *
 * The OrcCompiler structure has no public members
 */
struct _OrcCompiler {
  /*< private >*/
  OrcProgram *program;
  OrcTarget *target;

  unsigned int target_flags;

  OrcInstruction insns[ORC_N_INSNS];
  int n_insns;

  OrcVariable vars[ORC_N_COMPILER_VARIABLES];
  int n_temp_vars;
  int n_dup_vars;

  unsigned char *code;
  unsigned char *codeptr;
  
  OrcConstant constants[ORC_N_CONSTANTS];
  int n_constants;

  OrcFixup fixups[ORC_N_FIXUPS];
  int n_fixups;
  unsigned char *labels[ORC_N_LABELS];
  int labels_int[ORC_N_LABELS];
  int n_labels;

  int error;
  char *error_msg;
  OrcCompileResult result;

  int valid_regs[ORC_N_REGS];
  int save_regs[ORC_N_REGS];
  int used_regs[ORC_N_REGS];
  int alloc_regs[ORC_N_REGS];

  // 1 << loop shift means how many of these fit into a vector
  int loop_shift;
  int long_jumps;
  int use_frame_pointer;

  char *asm_code;
  int asm_code_len;

  int is_64bit;
  int tmpreg;
  int tmpreg2;
  int exec_reg;
  int gp_tmpreg;

  int insn_index;
  int unroll_index;
  int need_mask_regs;
  int unroll_shift;

  int alloc_loop_counter;
  int allow_gp_on_stack;
  int loop_counter;
  int size_region;
  int has_iterator_opcode;

  int offset;
  int min_temp_reg;
  int max_used_temp_reg;

  int insn_shift; /* used when emitting rules */
  int max_var_size; /* size of largest var */
  int load_params;

  void *output_insns;
  int n_output_insns;
  int n_output_insns_alloc;
  int temp_regs[ORC_N_REGS];
};

/* This is to differentiate between the case of code that has access
 * to the definition of OrcCompiler and the code that doesn't
 */
#ifdef ORC_SRC_ARG
#undef ORC_SRC_ARG
#define ORC_SRC_ARG(p,i,n) ((p)->vars[(i)->src_args[(n)]].alloc)
#endif

#ifdef ORC_DEST_ARG
#undef ORC_DEST_ARG
#define ORC_DEST_ARG(p,i,n) ((p)->vars[(i)->dest_args[(n)]].alloc)
#endif

#ifdef ORC_SRC_TYPE
#undef ORC_SRC_TYPE
#define ORC_SRC_TYPE(p,i,n) ((p)->vars[(i)->src_args[(n)]].vartype)
#endif

#ifdef ORC_DEST_TYPE
#undef ORC_DEST_TYPE
#define ORC_DEST_TYPE(p,i,n) ((p)->vars[(i)->dest_args[(n)]].vartype)
#endif

#ifdef ORC_SRC_VAL
#undef ORC_SRC_VAL
#define ORC_SRC_VAL(p,insn,n) ((p)->vars[(insn)->src_args[(n)]].value.i)
#endif

#ifdef ORC_DEST_VAL
#undef ORC_DEST_VAL
#define ORC_DEST_VAL(p,insn,n) ((p)->vars[(insn)->dest_args[(n)]].value.i)
#endif

/* orctarget.c */

/**
 * OrcTarget:
 *
 */
struct _OrcTarget {
  const char *name;
  orc_bool executable;
  int data_register_offset;

  unsigned int (*get_default_flags)(void);
  void (*compiler_init)(OrcCompiler *compiler);
  void (*compile)(OrcCompiler *compiler);

  OrcRuleSet rule_sets[ORC_N_RULE_SETS];
  int n_rule_sets;

  const char * (*get_asm_preamble)(void);
  void (*load_constant)(OrcCompiler *compiler, int reg, int size, int value);
  const char * (*get_flag_name)(int shift);
  void (*flush_cache) (OrcCode *code);
  void (*load_constant_long)(OrcCompiler *compiler, int reg,
      OrcConstant *constant);
  void *target_data;
  void *padding[4];
  /* Until here is for ABI compatibility for 0.4.41 */
  int register_size;
};

/* The function prototypes need to be visible to orc.c */
ORC_INTERNAL void orc_mmx_init (void);
ORC_INTERNAL void orc_sse_init (void);
ORC_INTERNAL void orc_avx_init (void);
ORC_INTERNAL void orc_arm_init (void);
ORC_INTERNAL void orc_powerpc_init (void);
ORC_INTERNAL void orc_c_init (void);
ORC_INTERNAL void orc_neon_init (void);
ORC_INTERNAL void orc_c64x_init (void);
ORC_INTERNAL void orc_c64x_c_init (void);
ORC_INTERNAL void orc_mips_init (void);
ORC_INTERNAL void orc_riscv_init (void);
ORC_INTERNAL void orc_lsx_init (void);
ORC_INTERNAL void orc_lasx_init (void);

typedef struct _OrcCodeRegion OrcCodeRegion;
typedef struct _OrcCodeChunk OrcCodeChunk;

/* This is internal API, nothing in the public headers returns an OrcCodeChunk
 */
OrcCodeRegion * orc_code_region_alloc (void);
void orc_code_chunk_free (OrcCodeChunk *chunk);

ORC_INTERNAL orc_bool orc_compiler_is_debug ();

ORC_INTERNAL extern int _orc_data_cache_size_level1;
ORC_INTERNAL extern int _orc_data_cache_size_level2;
ORC_INTERNAL extern int _orc_data_cache_size_level3;
ORC_INTERNAL extern int _orc_cpu_family;
ORC_INTERNAL extern int _orc_cpu_model;
ORC_INTERNAL extern int _orc_cpu_stepping;
ORC_INTERNAL extern const char *_orc_cpu_name;

ORC_INTERNAL void orc_compiler_emit_invariants (OrcCompiler *compiler);
ORC_INTERNAL int orc_compiler_has_float (OrcCompiler *compiler);

ORC_INTERNAL char* _orc_getenv (const char *var);
ORC_INTERNAL void orc_opcode_sys_init (void);

ORC_END_DECLS

#endif

