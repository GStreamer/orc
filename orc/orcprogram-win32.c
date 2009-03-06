
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>

#include <orc/orcprogram.h>

#define SIZE 65536


void
orc_program_allocate_codemem (OrcProgram *program)
{
  /* Now you know why Windows has viruses */

  program->code = malloc(SIZE);
  program->code_exec = program->code;
  program->code_size = SIZE;
  program->codeptr = program->code;
}

