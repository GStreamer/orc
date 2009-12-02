
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef HAVE_CODEMEM_MMAP
#include <sys/mman.h>
#endif

#include <orc/orcprogram.h>
#include <orc/orcdebug.h>


#define SIZE 65536


#ifdef HAVE_CODEMEM_MMAP
void
orc_compiler_allocate_codemem (OrcCompiler *compiler)
{
  int fd;
  int n;

  /* FIXME something combining these two would be ideal */
#if 1
  {
    char filename[32] = "/tmp/orcexecXXXXXX";
    fd = mkstemp (filename);
    if (fd == -1) {
      /* FIXME oh crap */
      ORC_COMPILER_ERROR (compiler, "failed to create temp file");
      return;
    }
    unlink (filename);
  }
#else
  {
    char *filename;

    filename = malloc (strlen ("/tmp/orcexec") +
        strlen (compiler->program->name) + 1);
    sprintf(filename, "/tmp/orcexec%s", compiler->program->name);
    fd = open (filename, O_RDWR | O_CREAT, S_IRWXU);
    if (fd == -1) {
      /* FIXME oh crap */
      ORC_COMPILER_ERROR (compiler, "failed to create temp file");
      return;
    }
    unlink (filename);
    free (filename);
  }
#endif

  n = ftruncate (fd, SIZE);

  compiler->program->code = mmap (NULL, SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
  if (compiler->program->code == MAP_FAILED) {
    ORC_COMPILER_ERROR(compiler, "failed to create write map");
    return;
  }
  compiler->program->code_exec = mmap (NULL, SIZE, PROT_READ|PROT_EXEC, MAP_SHARED, fd, 0);
  if (compiler->program->code_exec == MAP_FAILED) {
    ORC_COMPILER_ERROR(compiler, "failed to create exec map");
    return;
  }

  close (fd);

  compiler->program->code_size = SIZE;
  compiler->codeptr = compiler->program->code;
}
#endif

#ifdef HAVE_CODEMEM_MALLOC
void
orc_compiler_allocate_codemem (OrcCompiler *compiler)
{
  /* Now you know why Windows has viruses */

  compiler->program->code = malloc(SIZE);
  compiler->program->code_exec = compiler->program->code;
  compiler->program->code_size = SIZE;
  compiler->codeptr = compiler->program->code;
}
#endif

