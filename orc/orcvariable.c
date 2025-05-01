#include "config.h"

#include <orc/orcvariable.h>
#include <orc/orcdebug.h>

const char *
orc_variable_id_get_name (OrcVariableId id)
{
  static const char *names[] = {
    "d1", "d2", "d3", "d4",
    "s1", "s2", "s3", "s4",
    "s5", "s6", "s7", "s8",
    "a1", "a2", "a3", "d4",
    "c1", "c2", "c3", "c4",
    "c5", "c6", "c7", "c8",
    "p1", "p2", "p3", "p4",
    "p5", "p6", "p7", "p8",
    "t1", "t2", "t3", "t4",
    "t5", "t6", "t7", "t8",
    "t9", "t10", "t11", "t12",
    "t13", "t14", "t15", "t16"
  };

  if (id < 0 || id > ORC_VAR_T16)
    return NULL;

  return names[id];
}
