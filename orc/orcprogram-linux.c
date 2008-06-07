
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <orc/orcprogram.h>

#define SIZE 65536


void
orc_program_allocate_codemem (OrcProgram *program)
{
  char filename[32] = "/tmp/orcexecXXXXXX";
  int fd;

  fd = mkstemp (filename);
  if (fd == -1) {
    /* FIXME oh crap */
    printf("failed to create temp file\n");
    program->error = TRUE;
    return;
  }
  unlink (filename);

  ftruncate (fd, SIZE);

  program->code = mmap (NULL, SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
  if (program->code == MAP_FAILED) {
    /* FIXME oh crap */
    printf("failed to create write map\n");
    program->error = TRUE;
    return;
  }
  program->code_exec = mmap (NULL, SIZE, PROT_READ|PROT_EXEC, MAP_SHARED, fd, 0);
  if (program->code_exec == MAP_FAILED) {
    /* FIXME oh crap */
    printf("failed to create exec map\n");
    program->error = TRUE;
    return;
  }

  close (fd);

  program->code_size = SIZE;
  program->codeptr = program->code;
}

