
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <orc/orcprogram.h>

OrcType *types;
static int n_types;
static int n_alloc_types;

OrcType *
orc_type_get (const char *name)
{
  int i;
  for(i=0;i<n_types;i++){
    if (!strcmp (types[i].name, name)) {
      return types + i;
    }
  }
  return NULL;
}

void
orc_type_register (const char *name, int size)
{
  OrcType *type;

  if (n_types == n_alloc_types) {
    n_alloc_types += 100;
    types = realloc (types, sizeof(OrcType) * n_alloc_types);
  }

  type = types + n_types;
  type->name = strdup (name);
  type->size = size;

  n_types++;
}



