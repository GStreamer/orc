#pragma once

#include <orc/orcutils.h>

typedef struct _OrcX86Target OrcX86Target;
typedef struct _OrcTarget OrcTarget;

ORC_BEGIN_DECLS

ORC_INTERNAL void orc_x86_register_extension(OrcTarget *t, OrcX86Target *x86t);

ORC_END_DECLS
