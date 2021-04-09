
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
#include <sys/errno.h>
#endif

#ifdef HAVE_CODEMEM_VIRTUALALLOC
#include <windows.h>
  #ifdef ORC_WINAPI_ONLY_APP
    #define _virtualalloc VirtualAllocFromApp
  #else
    #define _virtualalloc VirtualAlloc
  #endif
#endif

#include <orc/orcinternal.h>
#include <orc/orcprogram.h>
#include <orc/orcdebug.h>


#define SIZE 65536

/* See _orc_compiler_init() */
extern int _orc_codemem_alignment;

typedef struct _OrcCodeRegion OrcCodeRegion;

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


static void orc_code_region_allocate_codemem (OrcCodeRegion *region);

static OrcCodeRegion **orc_code_regions;
static int orc_code_n_regions;


static OrcCodeRegion *
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

static OrcCodeChunk *
orc_code_chunk_split (OrcCodeChunk *chunk, int size)
{
  OrcCodeChunk *newchunk;

  newchunk = malloc(sizeof(OrcCodeChunk));
  memset (newchunk, 0, sizeof(OrcCodeChunk));

  newchunk->region = chunk->region;
  newchunk->offset = chunk->offset + size;
  newchunk->size = chunk->size - size;
  newchunk->next = chunk->next;
  newchunk->prev = chunk;

  chunk->size = size;
  if (chunk->next) {
    chunk->next->prev = newchunk;
  }
  chunk->next = newchunk;

  return newchunk;
}

static void
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

static OrcCodeChunk *
orc_code_region_get_free_chunk (int size)
{
  int i;
  OrcCodeRegion *region;
  OrcCodeChunk *chunk;

  orc_global_mutex_lock ();
  for(i=0;i<orc_code_n_regions;i++){
    region = orc_code_regions[i];
    for(chunk = region->chunks; chunk; chunk = chunk->next) {
      if (!chunk->used && size <= chunk->size) {
        orc_global_mutex_unlock ();
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
      orc_global_mutex_unlock ();
      return chunk;
    }
  }
  orc_global_mutex_unlock ();

  ORC_ASSERT(0);

  return NULL;
}

void
orc_code_allocate_codemem (OrcCode *code, int size)
{
  OrcCodeRegion *region;
  OrcCodeChunk *chunk;
  int aligned_size =
      (size + _orc_codemem_alignment) & (~_orc_codemem_alignment);

  chunk = orc_code_region_get_free_chunk (aligned_size);
  region = chunk->region;

  if (chunk->size > aligned_size) {
    orc_code_chunk_split (chunk, aligned_size);
  }

  chunk->used = TRUE;

  code->chunk = chunk;
  code->code = ORC_PTR_OFFSET(region->write_ptr, chunk->offset);
  code->exec = ORC_PTR_OFFSET(region->exec_ptr, chunk->offset);
  code->code_size = size;
  /* compiler->codeptr = ORC_PTR_OFFSET(region->write_ptr, chunk->offset); */
}

void
orc_code_chunk_free (OrcCodeChunk *chunk)
{
  if (_orc_compiler_flag_debug) {
    /* If debug is turned on, don't free code */
    return;
  }

  chunk->used = FALSE;
  if (chunk->next && !chunk->next->used) {
    orc_code_chunk_merge (chunk);
  }
  if (chunk->prev && !chunk->prev->used) {
    orc_code_chunk_merge (chunk->prev);
  }
}

#ifdef HAVE_CODEMEM_MMAP
static int
orc_code_region_allocate_codemem_dual_map (OrcCodeRegion *region,
    const char *dir, int force_unlink)
{
  int fd;
  int n;
  char *filename;
  mode_t mask;
  int exec_prot = PROT_READ | PROT_EXEC;

  if (_orc_compiler_flag_debug)
    exec_prot |= PROT_WRITE;

  filename = malloc (strlen ("/orcexec..") +
      strlen (dir) + 6 + 1);
  sprintf(filename, "%s/orcexec.XXXXXX", dir);
  mask = umask (0066);
  fd = mkstemp (filename);
  umask (mask);
  if (fd == -1) {
    ORC_WARNING ("failed to create temp file '%s'. err=%i", filename, errno);
    free (filename);
    return FALSE;
  }
  if (force_unlink || !_orc_compiler_flag_debug) {
    unlink (filename);
  }
  free (filename);

  n = ftruncate (fd, SIZE);
  if (n < 0) {
    ORC_WARNING("failed to expand file to size");
    close (fd);
    return FALSE;
  }

  region->exec_ptr = mmap (NULL, SIZE, exec_prot, MAP_SHARED, fd, 0);
  if (region->exec_ptr == MAP_FAILED) {
    ORC_WARNING("failed to create exec map '%s'. err=%i", filename, errno);
    close (fd);
    return FALSE;
  }
  region->write_ptr = mmap (NULL, SIZE, PROT_READ|PROT_WRITE,
      MAP_SHARED, fd, 0);
  if (region->write_ptr == MAP_FAILED) {
    ORC_WARNING ("failed to create write map '%s'. err=%i", filename, errno);
    munmap (region->exec_ptr, SIZE);
    close (fd);
    return FALSE;
  }
  region->size = SIZE;

  close (fd);
  return TRUE;
}

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

#ifndef MAP_JIT
#define MAP_JIT 0
#endif

static int
orc_code_region_allocate_codemem_anon_map (OrcCodeRegion *region)
{
  region->exec_ptr = mmap (NULL, SIZE, PROT_READ|PROT_WRITE|PROT_EXEC,
      MAP_PRIVATE|MAP_ANONYMOUS|MAP_JIT, -1, 0);
  if (region->exec_ptr == MAP_FAILED) {
    ORC_WARNING("failed to create write/exec map. err=%i", errno);
    return FALSE;
  }
  region->write_ptr = region->exec_ptr;
  region->size = SIZE;
  return TRUE;
}

static void
orc_code_region_allocate_codemem (OrcCodeRegion *region)
{
  const char *tmpdir;

  tmpdir = getenv ("XDG_RUNTIME_DIR");
  if (tmpdir && orc_code_region_allocate_codemem_dual_map (region,
        tmpdir, FALSE)) return;

  tmpdir = getenv ("HOME");
  if (tmpdir && orc_code_region_allocate_codemem_dual_map (region,
        tmpdir, FALSE)) return;

  tmpdir = getenv ("TMPDIR");
  if (tmpdir && orc_code_region_allocate_codemem_dual_map (region,
        tmpdir, FALSE)) return;

  if (orc_code_region_allocate_codemem_dual_map (region,
        "/tmp", FALSE)) return;

  if (orc_code_region_allocate_codemem_anon_map (region)) return;

#ifdef __APPLE__
  ORC_ERROR("Failed to create write and exec mmap regions.  This "
      "is probably because the Hardened Runtime is enabled without "
      "the com.apple.security.cs.allow-jit entitlement.");
#else
  ORC_ERROR("Failed to create write and exec mmap regions.  This "
      "is probably because SELinux execmem check is enabled (good) "
      "and $TMPDIR and $HOME are mounted noexec (bad).");
#endif
}

#endif

#ifdef HAVE_CODEMEM_VIRTUALALLOC
void
orc_code_region_allocate_codemem (OrcCodeRegion *region)
{
  /* On UWP, we can't allocate memory as executable from the start. We can only
   * set that later after compiling and copying the code over. This is a good
   * idea in general to avoid security issues, so we do it on win32 too. */
  region->write_ptr = _virtualalloc (NULL, SIZE, MEM_COMMIT, PAGE_READWRITE);
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

