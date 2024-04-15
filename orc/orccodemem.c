
#include "config.h"

#include <errno.h>
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


static int orc_code_region_allocate_codemem (OrcCodeRegion *region);

static OrcCodeRegion **orc_code_regions;
static int orc_code_n_regions;


OrcCodeRegion *
orc_code_region_alloc (void)
{
  OrcCodeRegion *region;

  region = malloc(sizeof(OrcCodeRegion));
  memset (region, 0, sizeof(OrcCodeRegion));

  if (!orc_code_region_allocate_codemem (region)) {
    free(region);
    return NULL;
  }

  return region;
}

static OrcCodeRegion *
orc_code_region_new (void)
{
  OrcCodeRegion *region;
  OrcCodeChunk *chunk;

  region = orc_code_region_alloc();

  if (!region) {
    return NULL;
  }

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

/* Must be called with orc_global_mutex_lock() */
static OrcCodeChunk *
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

  region = orc_code_region_new ();
  if (!region)
    return NULL;

  orc_code_regions = realloc (orc_code_regions,
      sizeof(void *)*(orc_code_n_regions+1));
  if (!orc_code_regions) {
    free(region);
    return NULL;
  }

  orc_code_regions[orc_code_n_regions] = region;
  orc_code_n_regions++;

  for(chunk = region->chunks; chunk; chunk = chunk->next) {
    if (!chunk->used && size <= chunk->size){
      return chunk;
    }
  }

  return NULL;
}

void
orc_code_allocate_codemem (OrcCode *code, int size)
{
  OrcCodeRegion *region;
  OrcCodeChunk *chunk;
  int aligned_size =
      (MAX(1, size) + _orc_codemem_alignment) & (~_orc_codemem_alignment);

  orc_global_mutex_lock ();
  chunk = orc_code_region_get_free_chunk (aligned_size);
  if (!chunk) {
    orc_global_mutex_unlock ();

    ORC_ERROR ("Failed to get free chunk memory");
    /* TODO: error out more gracefully? */
    ORC_ASSERT (0);
  }

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

  orc_global_mutex_unlock ();
}

void
orc_code_chunk_free (OrcCodeChunk *chunk)
{
  if (_orc_compiler_flag_debug) {
    /* If debug is turned on, don't free code */
    return;
  }

  orc_global_mutex_lock ();
  chunk->used = FALSE;
  if (chunk->next && !chunk->next->used) {
    orc_code_chunk_merge (chunk);
  }
  if (chunk->prev && !chunk->prev->used) {
    orc_code_chunk_merge (chunk->prev);
  }
  orc_global_mutex_unlock ();
}

#ifdef HAVE_CODEMEM_MMAP
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

#ifndef MAP_JIT
#define MAP_JIT 0
#endif

static int
orc_code_region_allocate_codemem (OrcCodeRegion *region)
{
  const char *msg = NULL;
  region->exec_ptr = mmap (NULL, SIZE, PROT_READ|PROT_WRITE,
    MAP_PRIVATE|MAP_ANONYMOUS|MAP_JIT, -1, 0);
  if (region->exec_ptr == MAP_FAILED) {
    msg = strerror(errno);
    ORC_ERROR("Failed to create mapping, err=%s", msg);
#ifdef __APPLE__
    ORC_ERROR("This is probably because the Hardened Runtime is enabled "
              "without the com.apple.security.cs.allow-jit entitlement."
              );
#else
    ORC_ERROR(
        "This is probably because SELinux execmem check is enabled (good), "
        "and anonymous mappings cannot be created (really bad)."
        );
#endif
    return FALSE;
  }
  region->write_ptr = region->exec_ptr;
  region->size = SIZE;
  return TRUE;
}
#elif defined(HAVE_CODEMEM_VIRTUALALLOC)
static int
orc_code_region_allocate_codemem (OrcCodeRegion *region)
{
  /* On UWP, we can't allocate memory as executable from the start. We can only
   * set that later after compiling and copying the code over. This is a good
   * idea in general to avoid security issues, so we do it on win32 too. */
  void *write_ptr;
  write_ptr = _virtualalloc (NULL, SIZE, MEM_COMMIT, PAGE_READWRITE);
  if (!write_ptr)
    return FALSE;

  region->write_ptr = write_ptr;
  region->exec_ptr = region->write_ptr;
  region->size = SIZE;
  return TRUE;
}
#elif defined(HAVE_CODEMEM_MALLOC)
static int
orc_code_region_allocate_codemem (OrcCodeRegion *region)
{
  void *write_ptr;
  write_ptr = malloc(SIZE);
  if (!write_ptr)
    return FALSE;

  region->write_ptr = write_ptr;
  region->exec_ptr = region->write_ptr;
  region->size = SIZE;
  return TRUE;
}
#endif

