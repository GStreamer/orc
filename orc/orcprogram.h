
#ifndef _ORC_PROGRAM_H_
#define _ORC_PROGRAM_H_

//#include <glib.h>

typedef struct _OrcType OrcType;
typedef struct _OrcExecutor OrcExecutor;
typedef struct _OrcVariable OrcVariable;
typedef struct _OrcOpcode OrcOpcode;
typedef struct _OrcArgument OrcArgument;
typedef struct _OrcInstruction OrcInstruction;
typedef struct _OrcProgram OrcProgram;
typedef struct _OrcRegister OrcRegister;
typedef struct _OrcRule OrcRule;
typedef struct _OrcFixup OrcFixup;

typedef void (*OrcOpcodeEmulateFunc)(OrcExecutor *ex, void *user);
typedef void (*OrcRuleEmitFunc)(OrcProgram *p, void *user, OrcInstruction *insn);

#define ORC_N_REGS (32*4)
#define ORC_N_INSNS 100
#define ORC_N_VARIABLES 100
#define ORC_N_REGISTERS 100
#define ORC_N_FIXUPS 100
#define ORC_N_LABELS 100

#define ORC_GP_REG_BASE 32
#define ORC_VEC_REG_BASE 64

#define ORC_REGCLASS_GP 1
#define ORC_REGCLASS_VEC 2

#define ORC_OPCODE_N_ARGS 4
#define ORC_OPCODE_N_RULES 12

#define ORC_STRUCT_OFFSET(struct_type, member)    \
      ((long) ((unsigned int *) &((struct_type*) 0)->member))

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

enum {
  ORC_RULE_C = 0,
  ORC_RULE_SCALAR_1,
  ORC_RULE_SCALAR_2,
  ORC_RULE_MMX_1,
  ORC_RULE_MMX_2,
  ORC_RULE_MMX_4,
  ORC_RULE_MMX_8,
  ORC_RULE_SSE_1,
  ORC_RULE_SSE_2,
  ORC_RULE_SSE_4,
  ORC_RULE_SSE_8,
  ORC_RULE_ALTIVEC_1
};

enum {
  ORC_TARGET_C = 0,
  ORC_TARGET_ALTIVEC = 1,
  ORC_TARGET_MMX = 2,
  ORC_TARGET_SSE = 3
};

struct _OrcType {
  char *name;
  int size;
};

typedef enum {
  ORC_VAR_TYPE_TEMP,
  ORC_VAR_TYPE_SRC,
  ORC_VAR_TYPE_DEST,
  ORC_VAR_TYPE_CONST,
  ORC_VAR_TYPE_PARAM
} OrcVarType;

struct _OrcVariable {
  char *name;

  OrcType *type;
  OrcVarType vartype;

  int used;
  int first_use;
  int last_use;
  int replaced;
  int replacement;

  int alloc;
  int is_chained;

  int16_t s16;
  unsigned char u8;

  int ptr_register;
  int ptr_offset;
};

struct _OrcRule {
  OrcRuleEmitFunc emit;
  void *emit_user;
};

struct _OrcOpcode {
  char *name;
  int n_src;
  int n_dest;
  OrcType *arg_types[ORC_OPCODE_N_ARGS];

  OrcRule rules[ORC_OPCODE_N_RULES];

  OrcOpcodeEmulateFunc emulate;
  void *emulate_user;
};

struct _OrcArgument {
  OrcVariable *var;
  int is_indirect;
  int is_indexed;
  OrcVariable *index_var;
  int index_scale;
  int type; // remove
  int index; // remove
  int offset;
};

struct _OrcInstruction {
  OrcOpcode *opcode;
  int args[3];

  OrcRule *rule;
  unsigned int rule_flag;
};

struct _OrcFixup {
  unsigned char *ptr;
  int type;
  int label;
};

struct _OrcRegister {
  int var;

  int is_data;
  int is_chained;
  int chained_reg;

  int merge;
};


struct _OrcProgram {
  OrcInstruction insns[ORC_N_INSNS];
  int n_insns;

  OrcVariable vars[ORC_N_VARIABLES];
  int n_vars;

  OrcInstruction *insn;
  int rule_set;

  OrcRegister registers[ORC_N_REGISTERS];
  int n_regs;

  unsigned char *code;
  void *code_exec;
  unsigned char *codeptr;
  int code_size;
  
  OrcFixup fixups[ORC_N_FIXUPS];
  int n_fixups;
  unsigned char *labels[ORC_N_LABELS];

  int error;

  int valid_regs[ORC_N_REGS];
  int save_regs[ORC_N_REGS];
  int used_regs[ORC_N_REGS];
  int alloc_regs[ORC_N_REGS];

  int target;
  int loop_shift;
  int n_per_loop;
};

struct _OrcExecutor {
  OrcProgram *program;
  int n;
  int counter1;
  int counter2;
  int counter3;

  void *arrays[ORC_N_VARIABLES];

  OrcVariable vars[ORC_N_VARIABLES];
  OrcVariable *args[ORC_OPCODE_N_ARGS];

};


void orc_init (void);

OrcProgram * orc_program_new (void);
OrcProgram * orc_program_new_dss (const char *type1, const char *type2,
    const char *type3);
OrcOpcode * orc_opcode_find_by_name (const char *name);
void orc_opcode_init (void);

void orc_program_append (OrcProgram *p, const char *opcode, int arg0, int arg1, int arg2);
void orc_program_append_str (OrcProgram *p, const char *opcode,
    const char * arg0, const char * arg1, const char * arg2);

void orc_mmx_init (void);
void orc_sse_init (void);
void orc_powerpc_init (void);
void orc_c_init (void);

void orc_program_set_target (OrcProgram *p, const char *target);

void orc_program_compile (OrcProgram *p);
void orc_program_c_init (OrcProgram *p);
void orc_program_mmx_init (OrcProgram *p);
void orc_program_sse_init (OrcProgram *p);
void orc_program_powerpc_init (OrcProgram *p);
void orc_program_mmx_assemble (OrcProgram *p);
void orc_program_sse_assemble (OrcProgram *p);
void orc_program_assemble_powerpc (OrcProgram *p);
void orc_program_assemble_c (OrcProgram *p);
void orc_program_free (OrcProgram *program);

int orc_program_find_var_by_name (OrcProgram *program, const char *name);

int orc_program_add_temporary (OrcProgram *program, const char *type, const char *name);
int orc_program_dup_temporary (OrcProgram *program, int i, int j);
int orc_program_add_source (OrcProgram *program, const char *type, const char *name);
int orc_program_add_destination (OrcProgram *program, const char *type, const char *name);
int orc_program_add_constant (OrcProgram *program, const char *type, int value, const char *name);
void orc_program_append (OrcProgram *program, const char *opcode, int arg0,
    int arg1, int arg2);

void orc_program_x86_reset_alloc (OrcProgram *program);
void orc_program_powerpc_reset_alloc (OrcProgram *program);


OrcType * orc_type_get (const char *name);
void orc_type_register (const char *name, int size);

OrcExecutor * orc_executor_new (OrcProgram *program);
void orc_executor_free (OrcExecutor *ex);
void orc_executor_set_array (OrcExecutor *ex, int var, void *ptr);
void orc_executor_set_array_str (OrcExecutor *ex, const char *name, void *ptr);
void orc_executor_set_n (OrcExecutor *ex, int n);
void orc_executor_emulate (OrcExecutor *ex);
void orc_executor_run (OrcExecutor *ex);

void orc_rule_register (const char *opcode_name, unsigned int mode,
    OrcRuleEmitFunc emit, void *emit_user);

int orc_program_allocate_register (OrcProgram *program, int is_data);
int orc_program_x86_allocate_register (OrcProgram *program, int is_data);
int orc_program_powerpc_allocate_register (OrcProgram *program, int is_data);

void orc_program_x86_register_rules (void);
void orc_program_allocate_codemem (OrcProgram *program);
void orc_program_dump_code (OrcProgram *program);

int orc_variable_get_size (OrcVariable *var);

void orc_program_append_code (OrcProgram *p, const char *fmt, ...);
 
#endif

