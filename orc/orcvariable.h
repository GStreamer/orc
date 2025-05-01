
#ifndef _ORC_VARIABLE_H_
#define _ORC_VARIABLE_H_

#include <orc/orcutils.h>

ORC_BEGIN_DECLS

#define ORC_VAR_FLAG_VOLATILE_WORKAROUND (1<<0)

typedef struct _OrcVariable OrcVariable;

typedef enum _OrcVariableId {
  /* Destination variables */
  ORC_VAR_D1,
  ORC_VAR_D2,
  ORC_VAR_D3,
  ORC_VAR_D4,
  /* Source variables */
  ORC_VAR_S1,
  ORC_VAR_S2,
  ORC_VAR_S3,
  ORC_VAR_S4,
  ORC_VAR_S5,
  ORC_VAR_S6,
  ORC_VAR_S7,
  ORC_VAR_S8,
  /* accumulators */
  // exec pointer (arrays) or m (params)
  ORC_VAR_A1,
  // OrcCode pointer (arrays) or m_index (params)
  ORC_VAR_A2,
  // elapsed time (params)
  ORC_VAR_A3,
  ORC_VAR_A4,
  /* constant variables */
  // row pointers (arrays [i+X]).
  // the stride for the array above is in params[i+X]
  ORC_VAR_C1,
  ORC_VAR_C2,
  ORC_VAR_C3,
  ORC_VAR_C4,
  ORC_VAR_C5,
  ORC_VAR_C6,
  ORC_VAR_C7,
  ORC_VAR_C8,
  /* parameter table index */
  ORC_VAR_P1,
  ORC_VAR_P2,
  ORC_VAR_P3,
  ORC_VAR_P4,
  ORC_VAR_P5,
  ORC_VAR_P6,
  ORC_VAR_P7,
  ORC_VAR_P8,
  /* temporary variables */
  // high half of params (params)
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
  ORC_VAR_T15,
  ORC_VAR_T16
} OrcVariableId;

typedef enum {
  ORC_VAR_TYPE_TEMP,
  ORC_VAR_TYPE_SRC,
  ORC_VAR_TYPE_DEST,
  ORC_VAR_TYPE_CONST,
  ORC_VAR_TYPE_PARAM,
  ORC_VAR_TYPE_ACCUMULATOR
} OrcVarType;

typedef enum _OrcParamType {
  ORC_PARAM_TYPE_INT = 0,
  ORC_PARAM_TYPE_FLOAT,
  ORC_PARAM_TYPE_INT64,
  ORC_PARAM_TYPE_DOUBLE
} OrcParamType;


/**
 * OrcVariable:
 *
 * The OrcVariable structure has no public members
 */
struct _OrcVariable {
  /*< private >*/
  char *name;
  char *type_name;

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
  int alignment;
  int is_uncached;

  orc_union64 value;

  int ptr_register;
  int ptr_offset;
  int mask_alloc;
  int aligned_data;
  int param_type;
  int load_dest;
  int update_type;
  int need_offset_reg;
  unsigned int flags;

  int has_parameter;
  int parameter;
};

#ifdef ORC_ENABLE_UNSTABLE_API

ORC_API const char * orc_variable_id_get_name (OrcVariableId id);

#endif

ORC_END_DECLS

#endif

