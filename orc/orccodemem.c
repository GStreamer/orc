
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
#endif

#ifdef HAVE_CODEMEM_VIRTUALALLOC
#include <windows.h>
  #ifdef ORC_WINAPI_ONLY_APP
    #define _virtualalloc VirtualAllocFromApp
  #else
    #define _virtualalloc VirtualAlloc
  #endif
#endif

#include <orc/orcprogram.h>
#include <orc/orcinternal.h>
#include <orc/orcutils-private.h>
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

  region = orc_malloc(sizeof(OrcCodeRegion));
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

  chunk = orc_malloc(sizeof(OrcCodeChunk));
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

  newchunk = orc_malloc(sizeof(OrcCodeChunk));
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
    return;
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
  if (orc_compiler_is_debug ()) {
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
static int
orc_code_region_allocate_codemem_dual_map (OrcCodeRegion *region,
    const char *dir, int force_unlink)
{
  int fd;
  int n;
  char *filename;
  int exec_prot = PROT_READ | PROT_EXEC;

  if (orc_compiler_is_debug ())
    exec_prot |= PROT_WRITE;

  filename = malloc (strlen ("/orcexec..") +
      strlen (dir) + 6 + 1);

  if (filename == NULL)
    return FALSE;

  sprintf(filename, "%s/orcexec.XXXXXX", dir);
  fd = mkstemp (filename);
  if (fd == -1) {
    ORC_WARNING ("failed to create temp file '%s'. err=%i", filename, errno);
    free (filename);
    return FALSE;
  }
  if (force_unlink || !orc_compiler_is_debug ()) {
    unlink (filename);
  }

  n = ftruncate (fd, SIZE);
  if (n < 0) {
    ORC_WARNING("failed to expand file to size");
    close (fd);
    free (filename);
    return FALSE;
  }

  region->exec_ptr = mmap (NULL, SIZE, exec_prot, MAP_SHARED, fd, 0);
  if (region->exec_ptr == MAP_FAILED) {
    ORC_WARNING("failed to create exec map '%s'. err=%i", filename, errno);
    close (fd);
    free (filename);
    return FALSE;
  }
  region->write_ptr = mmap (NULL, SIZE, PROT_READ|PROT_WRITE,
      MAP_SHARED, fd, 0);
  if (region->write_ptr == MAP_FAILED) {
    ORC_WARNING ("failed to create write map '%s'. err=%i", filename, errno);
    free (filename);
    munmap (region->exec_ptr, SIZE);
    close (fd);
    return FALSE;
  }
  region->size = SIZE;

  free (filename);
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

int
orc_code_region_allocate_codemem (OrcCodeRegion *region)
{
  const char *tmpdir;

  tmpdir = getenv ("XDG_RUNTIME_DIR");
  if (tmpdir && orc_code_region_allocate_codemem_dual_map (region,
        tmpdir, FALSE)) return TRUE;

  tmpdir = getenv ("HOME");
  if (tmpdir && orc_code_region_allocate_codemem_dual_map (region,
        tmpdir, FALSE)) return TRUE;

  tmpdir = getenv ("TMPDIR");
  if (tmpdir && orc_code_region_allocate_codemem_dual_map (region,
        tmpdir, FALSE)) return TRUE;

  if (orc_code_region_allocate_codemem_dual_map (region,
        "/tmp", FALSE)) return TRUE;

  if (orc_code_region_allocate_codemem_anon_map (region)) return TRUE;

#ifdef __APPLE__
  ORC_ERROR("Failed to create write and exec mmap regions.  This "
      "is probably because the Hardened Runtime is enabled without "
      "the com.apple.security.cs.allow-jit entitlement.");
#else
  ORC_ERROR(
      "Failed to create write+exec mappings. This "
      "is probably because SELinux execmem check is enabled (good), "
      "$XDG_RUNTIME_DIR, $HOME, $TMPDIR, $HOME and /tmp are mounted noexec (good), "
      "and anonymous mappings cannot be created (really bad)."
      );
#endif
  return FALSE;
}

#endif

#ifdef HAVE_CODEMEM_VIRTUALALLOC
int
orc_code_region_allocate_codemem (OrcCodeRegion *region)
{
  /* On UWP, we can't allocate memory as executable from the start. We can only
   * set that later after compiling and copying the code over. This is a good
   * idea in general to avoid security issues, so we do it on win32 too. */
  void *write_ptr = NULL;
  char *msg = NULL;
  write_ptr = _virtualalloc (NULL, SIZE, MEM_COMMIT, PAGE_READWRITE);
  if (!write_ptr) {
    FormatMessageA (FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, GetLastError(), 0, (LPTSTR)&msg, 0, NULL);
    ORC_ERROR ("Couldn't allocate mapping on %p of size %d: %s", region,
        SIZE, msg);
    LocalFree (msg);
    return FALSE;
  }

  region->write_ptr = write_ptr;
  region->exec_ptr = region->write_ptr;
  region->size = SIZE;
  return TRUE;
}
#endif

#ifdef HAVE_CODEMEM_MALLOC
int
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

