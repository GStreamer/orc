
#ifndef _ORC_PROGRAM_H_
#define _ORC_PROGRAM_H_

#include <orc/orc-stdint.h>
#include <orc/orcutils.h>

typedef struct _OrcOpcodeExecutor OrcOpcodeExecutor;
typedef struct _OrcExecutor OrcExecutor;
typedef struct _OrcVariable OrcVariable;
typedef struct _OrcOpcodeSet OrcOpcodeSet;
typedef struct _OrcStaticOpcode OrcStaticOpcode;
typedef struct _OrcInstruction OrcInstruction;
typedef struct _OrcProgram OrcProgram;
typedef struct _OrcCompiler OrcCompiler;
typedef struct _OrcRule OrcRule;
typedef struct _OrcRuleSet OrcRuleSet;
typedef struct _OrcConstant OrcConstant;
typedef struct _OrcFixup OrcFixup;
typedef struct _OrcTarget OrcTarget;

typedef void (*OrcOpcodeEmulateFunc)(OrcOpcodeExecutor *ex, void *user);
typedef void (*OrcRuleEmitFunc)(OrcCompiler *p, void *user, OrcInstruction *insn);

#define ORC_N_REGS (32*4)
#define ORC_N_INSNS 100
#define ORC_N_VARIABLES 64
#define ORC_N_REGISTERS 20
#define ORC_N_FIXUPS 20
#define ORC_N_CONSTANTS 20
#define ORC_N_LABELS 20

#define ORC_GP_REG_BASE 32
#define ORC_VEC_REG_BASE 64

#define ORC_REGCLASS_GP 1
#define ORC_REGCLASS_VEC 2

#define ORC_STATIC_OPCODE_N_SRC 4
#define ORC_STATIC_OPCODE_N_DEST 2

#define ORC_OPCODE_N_ARGS 4
#define ORC_N_TARGETS 10
#define ORC_N_RULE_SETS 10

#define ORC_STRUCT_OFFSET(struct_type, member)    \
      ((long) ((unsigned int *) &((struct_type*) 0)->member))

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define ORC_ENABLE_ASM_CODE
#ifdef ORC_ENABLE_ASM_CODE
#define ORC_ASM_CODE(compiler,...) orc_compiler_append_code(compiler, __VA_ARGS__)
#else
#define ORC_ASM_CODE(compiler,...)
#endif

#define ORC_PROGRAM_ERROR(program, ...) do { \
  program->error = TRUE; \
  orc_debug_print(ORC_DEBUG_ERROR, __FILE__, ORC_FUNCTION, __LINE__, __VA_ARGS__); \
} while (0)

#define ORC_COMPILER_ERROR(compiler, ...) do { \
  compiler->error = TRUE; \
  compiler->result = ORC_COMPILE_RESULT_UNKNOWN_PARSE; \
  orc_debug_print(ORC_DEBUG_ERROR, __FILE__, ORC_FUNCTION, __LINE__, __VA_ARGS__); \
} while (0)

enum {
  ORC_TARGET_C_C99 = (1<<0)
};

enum {
  ORC_TARGET_SSE_SSE2 = (1<<0),
  ORC_TARGET_SSE_SSE3 = (1<<1),
  ORC_TARGET_SSE_SSSE3 = (1<<2),
  ORC_TARGET_SSE_SSE4_1 = (1<<3),
  ORC_TARGET_SSE_SSE4_2 = (1<<4),
  ORC_TARGET_SSE_SSE4A = (1<<5),
  ORC_TARGET_SSE_SSE5 = (1<<6)
};
enum {
  ORC_TARGET_ALTIVEC_ALTIVEC = (1<<0)
};

typedef enum {
  ORC_VAR_TYPE_TEMP,
  ORC_VAR_TYPE_SRC,
  ORC_VAR_TYPE_DEST,
  ORC_VAR_TYPE_CONST,
  ORC_VAR_TYPE_PARAM,
  ORC_VAR_TYPE_ACCUMULATOR
} OrcVarType;

enum {
  ORC_VAR_D1,
  ORC_VAR_D2,
  ORC_VAR_D3,
  ORC_VAR_D4,
  ORC_VAR_S1,
  ORC_VAR_S2,
  ORC_VAR_S3,
  ORC_VAR_S4,
  ORC_VAR_S5,
  ORC_VAR_S6,
  ORC_VAR_S7,
  ORC_VAR_S8,
  ORC_VAR_A1,
  ORC_VAR_A2,
  ORC_VAR_A3,
  ORC_VAR_A4,
  ORC_VAR_C1,
  ORC_VAR_C2,
  ORC_VAR_C3,
  ORC_VAR_C4,
  ORC_VAR_C5,
  ORC_VAR_C6,
  ORC_VAR_C7,
  ORC_VAR_C8,
  ORC_VAR_P1,
  ORC_VAR_P2,
  ORC_VAR_P3,
  ORC_VAR_P4,
  ORC_VAR_P5,
  ORC_VAR_P6,
  ORC_VAR_P7,
  ORC_VAR_P8,
  ORC_VAR_T1,
  ORC_VAR_T2,
  ORC_VAR_T3,
  ORC_VAR_T4,
  ORC_VAR_T5,
  ORC_VAR_T6,
  ORC_VAR_T7,
  ORC_VAR_T8,
  ORC_VAR_T9,
  ORC_VAR_T10,
  ORC_VAR_T11,
  ORC_VAR_T12,
  ORC_VAR_T13,
  ORC_VAR_T14,
  ORC_VAR_T15
};

enum {
  ORC_CONST_ZERO,
  ORC_CONST_SPLAT_B,
  ORC_CONST_SPLAT_W,
  ORC_CONST_SPLAT_L,
};

typedef enum {
  ORC_COMPILE_RESULT_OK = 0,

  ORC_COMPILE_RESULT_UNKNOWN_COMPILE = 0x100,
  ORC_COMPILE_RESULT_MISSING_RULE = 0x101,

  ORC_COMPILE_RESULT_UNKNOWN_PARSE = 0x200,
  ORC_COMPILE_RESULT_PARSE = 0x201,
  ORC_COMPILE_RESULT_VARIABLE = 0x202

} OrcCompileResult;

#define ORC_COMPILE_RESULT_IS_SUCCESSFUL(x) ((x) < 0x100)
#define ORC_COMPILE_RESULT_IS_FATAL(x) ((x) >= 0x200)

struct _OrcVariable {
  char *name;

  int size;
  OrcVarType vartype;

  int used;
  int first_use;
  int last_use;
  int replaced;
  int replacement;

  int alloc;
  int is_chained;
  int is_aligned;
  int is_uncached;

  int value;

  int ptr_register;
  int ptr_offset;
};

struct _OrcRule {
  OrcRuleEmitFunc emit;
  void *emit_user;
};

struct _OrcRuleSet {
  OrcOpcodeSet *opcode_set;
  int required_target_flags;

  OrcRule *rules;
  int n_rules;
};

struct _OrcOpcodeSet {
  int opcode_major;
  char prefix[8];

  int n_opcodes;
  OrcStaticOpcode *opcodes;
};

#define ORC_STATIC_OPCODE_ACCUMULATOR 1

struct _OrcStaticOpcode {
  char name[16];
  OrcOpcodeEmulateFunc emulate;
  void *emulate_user;
  unsigned int flags;
  int dest_size[ORC_STATIC_OPCODE_N_DEST];
  int src_size[ORC_STATIC_OPCODE_N_SRC];
};

struct _OrcInstruction {
  OrcStaticOpcode *opcode;
  int dest_args[ORC_STATIC_OPCODE_N_DEST];
  int src_args[ORC_STATIC_OPCODE_N_SRC];

  OrcRule *rule;
};

struct _OrcConstant {
  int type;
  int alloc_reg;
  unsigned int value;
  unsigned int full_value[4];
};

struct _OrcFixup {
  unsigned char *ptr;
  int type;
  int label;
};

struct _OrcProgram {
  OrcInstruction insns[ORC_N_INSNS];
  int n_insns;

  OrcVariable vars[ORC_N_VARIABLES];
  //int n_vars;
  int n_src_vars;
  int n_dest_vars;
  int n_param_vars;
  int n_const_vars;
  int n_temp_vars;
  int n_accum_vars;

  char *name;
  char *asm_code;

  unsigned char *code;
  void *code_exec;
  int code_size;
};

struct _OrcCompiler {
  OrcProgram *program;
  OrcTarget *target;

  unsigned int target_flags;

  OrcInstruction insns[ORC_N_INSNS];
  int n_insns;

  OrcVariable vars[ORC_N_VARIABLES];
  //int n_vars;
  int n_temp_vars;

  unsigned char *codeptr;
  
  OrcConstant constants[ORC_N_CONSTANTS];
  int n_constants;

  OrcFixup fixups[ORC_N_FIXUPS];
  int n_fixups;
  unsigned char *labels[ORC_N_LABELS];
  int n_labels;

  int error;
  OrcCompileResult result;

  int valid_regs[ORC_N_REGS];
  int save_regs[ORC_N_REGS];
  int used_regs[ORC_N_REGS];
  int alloc_regs[ORC_N_REGS];

  int loop_shift;
  int long_jumps;
  int use_frame_pointer;

  char *asm_code;
  int asm_code_len;

  int is_64bit;
  int tmpreg;
  int exec_reg;
  int gp_tmpreg;
};

struct _OrcOpcodeExecutor {
  int src_values[ORC_STATIC_OPCODE_N_SRC];
  int dest_values[ORC_STATIC_OPCODE_N_DEST];
};

struct _OrcExecutor {
  OrcProgram *program;
  int n;
  int counter1;
  int counter2;
  int counter3;

  void *arrays[ORC_N_VARIABLES];
  int params[ORC_N_VARIABLES];
  int accumulators[4];
};

struct _OrcTarget {
  const char *name;
  orc_bool executable;
  int data_register_offset;

  void (*compiler_init)(OrcCompiler *compiler);
  void (*compile)(OrcCompiler *compiler);

  OrcRuleSet rule_sets[ORC_N_RULE_SETS];
  int n_rule_sets;
};


void orc_init (void);

OrcProgram * orc_program_new (void);
OrcProgram * orc_program_new_ds (int size1, int size2);
OrcProgram * orc_program_new_dss (int size1, int size2, int size3);
OrcProgram * orc_program_new_as (int size1, int size2);
OrcProgram * orc_program_new_ass (int size1, int size2, int size3);
OrcStaticOpcode * orc_opcode_find_by_name (const char *name);
void orc_opcode_init (void);

const char * orc_program_get_name (OrcProgram *program);
void orc_program_set_name (OrcProgram *program, const char *name);

void orc_program_append (OrcProgram *p, const char *opcode, int arg0, int arg1, int arg2);
void orc_program_append_str (OrcProgram *p, const char *opcode,
    const char * arg0, const char * arg1, const char * arg2);
void orc_program_append_ds (OrcProgram *program, const char *opcode, int arg0,
    int arg1);
void orc_program_append_ds_str (OrcProgram *p, const char *opcode,
    const char * arg0, const char * arg1);

void orc_mmx_init (void);
void orc_sse_init (void);
void orc_arm_init (void);
void orc_powerpc_init (void);
void orc_c_init (void);

OrcCompileResult orc_program_compile (OrcProgram *p);
OrcCompileResult orc_program_compile_for_target (OrcProgram *p, OrcTarget *target);
void orc_program_free (OrcProgram *program);

int orc_program_find_var_by_name (OrcProgram *program, const char *name);
int orc_compiler_get_dest (OrcCompiler *compiler);

int orc_program_add_temporary (OrcProgram *program, int size, const char *name);
int orc_program_dup_temporary (OrcProgram *program, int i, int j);
int orc_program_add_source (OrcProgram *program, int size, const char *name);
int orc_program_add_destination (OrcProgram *program, int size, const char *name);
int orc_program_add_constant (OrcProgram *program, int size, int value, const char *name);
int orc_program_add_parameter (OrcProgram *program, int size, const char *name);
int orc_program_add_accumulator (OrcProgram *program, int size, const char *name);

void orc_program_x86_reset_alloc (OrcProgram *program);
void orc_program_powerpc_reset_alloc (OrcProgram *program);


OrcExecutor * orc_executor_new (OrcProgram *program);
void orc_executor_free (OrcExecutor *ex);
void orc_executor_set_program (OrcExecutor *ex, OrcProgram *program);
void orc_executor_set_array (OrcExecutor *ex, int var, void *ptr);
void orc_executor_set_array_str (OrcExecutor *ex, const char *name, void *ptr);
void orc_executor_set_param (OrcExecutor *ex, int var, int value);
void orc_executor_set_param_str (OrcExecutor *ex, const char *name, int value);
int orc_executor_get_accumulator (OrcExecutor *ex, int var);
int orc_executor_get_accumulator_str (OrcExecutor *ex, const char *name);
void orc_executor_set_n (OrcExecutor *ex, int n);
void orc_executor_emulate (OrcExecutor *ex);
void orc_executor_run (OrcExecutor *ex);

OrcOpcodeSet *orc_opcode_set_get (const char *name);
int orc_opcode_set_find_by_name (OrcOpcodeSet *opcode_set, const char *name);
int orc_opcode_register_static (OrcStaticOpcode *sopcode, char *prefix);

OrcRuleSet * orc_rule_set_new (OrcOpcodeSet *opcode_set, OrcTarget *target,
    unsigned int required_flags);
void orc_rule_register (OrcRuleSet *rule_set, const char *opcode_name,
    OrcRuleEmitFunc emit, void *emit_user);
OrcRule * orc_target_get_rule (OrcTarget *target, OrcStaticOpcode *opcode,
    unsigned int target_flags);
OrcTarget * orc_target_get_default (void);

int orc_program_allocate_register (OrcProgram *program, int is_data);
int orc_program_x86_allocate_register (OrcProgram *program, int is_data);
int orc_program_powerpc_allocate_register (OrcProgram *program, int is_data);

void orc_program_x86_register_rules (void);
void orc_compiler_allocate_codemem (OrcCompiler *compiler);
void orc_program_dump_code (OrcProgram *program);
int orc_compiler_label_new (OrcCompiler *compiler);

const char *orc_program_get_asm_code (OrcProgram *program);
void orc_program_dump_asm (OrcProgram *program);
const char *orc_target_get_asm_preamble (const char *target);

void orc_compiler_append_code (OrcCompiler *p, const char *fmt, ...)
  ORC_GNU_PRINTF(2,3);
 
void orc_target_register (OrcTarget *target);
OrcTarget *orc_target_get_by_name (const char *target_name);
int orc_program_get_max_var_size (OrcProgram *program);

#endif

