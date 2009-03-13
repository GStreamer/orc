
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <orc/orcprogram.h>
#include <orc/orcdebug.h>

#define SIZE 65536


void
orc_program_allocate_codemem (OrcProgram *program)
{
  char filename[32] = "/tmp/orcexecXXXXXX";
  int fd;
  int n;

  fd = mkstemp (filename);
  if (fd == -1) {
    /* FIXME oh crap */
    ORC_ERROR ("failed to create temp file");
    program->error = TRUE;
    return;
  }
  unlink (filename);

  n = ftruncate (fd, SIZE);

  program->code = mmap (NULL, SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
  if (program->code == MAP_FAILED) {
    /* FIXME oh crap */
    ORC_ERROR ("failed to create write map");
    program->error = TRUE;
    return;
  }
  program->code_exec = mmap (NULL, SIZE, PROT_READ|PROT_EXEC, MAP_SHARED, fd, 0);
  if (program->code_exec == MAP_FAILED) {
    /* FIXME oh crap */
    ORC_ERROR ("failed to create exec map");
    program->error = TRUE;
    return;
  }

  close (fd);

  program->code_size = SIZE;
  program->codeptr = program->code;
}

