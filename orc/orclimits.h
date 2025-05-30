
#ifndef _ORC_LIMITS_H_
#define _ORC_LIMITS_H_

#include <orc/orcutils.h>

ORC_BEGIN_DECLS

#define ORC_N_REGS (32*4)
#define ORC_N_INSNS 100
#define ORC_N_VARIABLES 64
#define ORC_N_ARRAYS 12
#define ORC_N_REGISTERS 20
#define ORC_N_FIXUPS 100
#define ORC_N_CONSTANTS 20
#define ORC_N_LABELS 40
#define ORC_N_COMPILER_VARIABLES (ORC_N_VARIABLES+32)
#define ORC_N_PARAMS 8

#define ORC_GP_REG_BASE 32
#define ORC_VEC_REG_BASE 64
#define ORC_REG_INVALID 0

#define ORC_STATIC_OPCODE_N_SRC 4
#define ORC_STATIC_OPCODE_N_DEST 2

#define ORC_OPCODE_N_ARGS 4
#define ORC_N_TARGETS 10
#define ORC_N_RULE_SETS 10

#define ORC_MAX_VAR_SIZE 8

#define ORC_MAX_DEST_VARS 4
#define ORC_MAX_SRC_VARS 8
#define ORC_MAX_TEMP_VARS 16
#define ORC_MAX_CONST_VARS 8
#define ORC_MAX_PARAM_VARS 8
#define ORC_MAX_ACCUM_VARS 4

ORC_END_DECLS

#endif

