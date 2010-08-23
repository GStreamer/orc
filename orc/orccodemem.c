
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
typedef struct _OrcCodeChunk OrcCodeChunk;

struct _OrcCodeRegion {
  orc_uint8 *write_ptr;
  orc_uint8 *exec_ptr;
  int size;

  OrcCodeChunk *chunks;
};

struct _OrcCodeChunk {
  /*< private >*/
  struct _OrcCodeChunk *next;
  struct _OrcCodeChunk *prev;
  struct _OrcCodeRegion *region;
  int used;

  int offset;
  int size;
};


void orc_code_region_allocate_codemem (OrcCodeRegion *region);

static OrcCodeRegion **orc_code_regions;
static int orc_code_n_regions;


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
  chunk->used = FALSE;
  chunk->region = region;
  chunk->size = region->size;

  region->chunks = chunk;

  return region;
}

OrcCodeChunk *
orc_code_chunk_split (OrcCodeChunk *chunk, int size)
{
  OrcCodeChunk *newchunk;

  newchunk = malloc(sizeof(OrcCodeChunk));
  memset (newchunk, 0, sizeof(OrcCodeChunk));

  newchunk->region = chunk->region;
  newchunk->offset = chunk->offset + size;
  newchunk->size = chunk->size - size;
  newchunk->next = chunk->next;
  newchunk->prev = chunk->prev;

  chunk->size = size;
  if (chunk->next) {
    chunk->next->prev = newchunk;
  }
  chunk->next = newchunk;

  return newchunk;
}

void
orc_code_chunk_merge (OrcCodeChunk *chunk)
{
  OrcCodeChunk *chunk2 = chunk->next;

  chunk->next = chunk2->next;
  if (chunk2->next) {
    chunk2->next->prev = chunk;
  }
  chunk->size += chunk2->size;

  free(chunk2);
}

OrcCodeChunk *
orc_code_region_get_free_chunk (int size)
{
  int i;
  OrcCodeRegion *region;
  OrcCodeChunk *chunk;

  for(i=0;i<orc_code_n_regions;i++){
    region = orc_code_regions[i];
    for(chunk = region->chunks; chunk; chunk = chunk->next) {
      if (!chunk->used && size <= chunk->size) {
        return chunk;
      }
    }
  }

  orc_code_regions = realloc (orc_code_regions,
      sizeof(void *)*(orc_code_n_regions+1));
  orc_code_regions[orc_code_n_regions] = orc_code_region_new ();
  region = orc_code_regions[orc_code_n_regions];
  orc_code_n_regions++;

  for(chunk = region->chunks; chunk; chunk = chunk->next) {
    if (!chunk->used && size <= chunk->size){
      return chunk;
    }
  }

  ORC_ASSERT(0);

  return NULL;
}

void
orc_compiler_allocate_codemem (OrcCompiler *compiler)
{
  OrcCodeRegion *region;
  OrcCodeChunk *chunk;
  int size = 4096;

  chunk = orc_code_region_get_free_chunk (size);
  region = chunk->region;

  if (chunk->size > size) {
    orc_code_chunk_split (chunk, size);
  }

  chunk->used = TRUE;

  compiler->program->code = ORC_PTR_OFFSET(region->write_ptr, chunk->offset);
  compiler->program->code_exec = ORC_PTR_OFFSET(region->exec_ptr, chunk->offset);
  compiler->program->code_size = chunk->size;
  compiler->codeptr = ORC_PTR_OFFSET(region->write_ptr, chunk->offset);
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

