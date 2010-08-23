
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
#ifdef HAVE_CODEMEM_VIRTUALALLOC
#include <windows.h>
#endif

#include <orc/orcprogram.h>
#include <orc/orcdebug.h>


#define SIZE 65536

typedef struct _OrcCodeRegion OrcCodeRegion;
struct _OrcCodeRegion {
  orc_uint8 *write_ptr;
  orc_uint8 *exec_ptr;
  int size;

  OrcCodeChunk *used_chunks;
  OrcCodeChunk *free_chunks;
};

void orc_code_region_allocate_codemem (OrcCodeRegion *region);

OrcCodeRegion *orc_code_regions;
int n_orc_code_regions;


OrcCodeRegion *
orc_code_region_new (void)
{
  OrcCodeRegion *region;
  OrcCodeChunk *chunk;

  region = malloc(sizeof(OrcCodeRegion));
  memset (region, 0, sizeof(OrcCodeRegion));

  orc_code_region_allocate_codemem (region);

  chunk = malloc(sizeof(OrcCodeChunk));
  memset (chunk, 0, sizeof(OrcCodeChunk));

  chunk->offset = 0;
  chunk->region = region;
  chunk->write_ptr = ORC_PTR_OFFSET (region->write_ptr, chunk->offset);
  chunk->exec_ptr = ORC_PTR_OFFSET (region->exec_ptr, chunk->offset);
  chunk->size = region->size;

  region->free_chunks = chunk;

  return region;
}

void
orc_compiler_allocate_codemem (OrcCompiler *compiler)
{
  OrcCodeRegion *region;
  OrcCodeChunk *chunk;

  region = orc_code_region_new ();

  chunk = region->free_chunks;
  region->free_chunks = NULL;
  region->used_chunks = chunk;

  compiler->program->code = chunk->write_ptr;
  compiler->program->code_exec = chunk->exec_ptr;
  compiler->program->code_size = chunk->size;
  compiler->codeptr = chunk->write_ptr;

}




#ifdef HAVE_CODEMEM_MMAP
void
orc_code_region_allocate_codemem (OrcCodeRegion *region)
{
  int fd;
  int n;
  static char *tmpdir = NULL;
  char *filename;

  if (tmpdir == NULL) {
    tmpdir = getenv ("TMPDIR");
    if (tmpdir == NULL) {
      tmpdir = "/tmp";
    }
  }

#if 0
  filename = malloc (strlen ("/orcexec..") +
      strlen (tmpdir) + strlen (compiler->program->name) + 6 + 1);
  sprintf(filename, "%s/orcexec.%s.XXXXXX", tmpdir, compiler->program->name);
#endif
  filename = malloc (strlen ("/orcexec..") +
      strlen (tmpdir) + 6 + 1);
  sprintf(filename, "%s/orcexec.XXXXXX", tmpdir);
  fd = mkstemp (filename);
  if (fd == -1) {
    /* FIXME oh crap */
    ORC_ERROR ("failed to create temp file");
    return;
  }
  if (!_orc_compiler_flag_debug) {
    unlink (filename);
  }
  free (filename);

  n = ftruncate (fd, SIZE);

  region->write_ptr = mmap (NULL, SIZE, PROT_READ|PROT_WRITE,
      MAP_SHARED, fd, 0);
  if (region->write_ptr == MAP_FAILED) {
    ORC_ERROR("failed to create write map");
    return;
  }
  region->exec_ptr = mmap (NULL, SIZE, PROT_READ|PROT_EXEC,
      MAP_SHARED, fd, 0);
  if (region->exec_ptr == MAP_FAILED) {
    ORC_ERROR("failed to create exec map");
    return;
  }
  region->size = SIZE;

  close (fd);
}
#endif

#ifdef HAVE_CODEMEM_VIRTUALALLOC
void
orc_code_region_allocate_codemem (OrcCodeRegion *region)
{
  region->write_ptr = VirtualAlloc(NULL, SIZE, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
  region->exec_ptr = region->write_ptr;
  region->size = SIZE;
}
#endif

#ifdef HAVE_CODEMEM_MALLOC
void
orc_code_region_allocate_codemem (OrcCodeRegion *region)
{
  region->write_ptr = malloc(SIZE);
  region->exec_ptr = region->write_ptr;
  region->size = SIZE;
}
#endif

