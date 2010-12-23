
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <orc/orc.h>

#include <stdio.h>


int main (int argc, char *argv[])
{
  long offset;
  int expected_offset;
  int error = 0;

  offset = ORC_STRUCT_OFFSET (OrcProgram, code_exec);

  if (sizeof(void *) == 4) {
    expected_offset = 8360;
  } else {
    expected_offset = 9688;
  }

  if (offset != expected_offset) {
    printf("ABI bug: OrcProgram->code_exec should be at offset %ld instead of %d\n",
        offset, expected_offset);
    error = 1;
  }

  return error;
}

