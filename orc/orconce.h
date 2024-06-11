
#ifndef _ORC_ONCE_H_
#define _ORC_ONCE_H_

#include <orc/orcutils.h>
#include <orc/orcdebug.h>

#if (!defined(_MSC_VER) || defined(__clang__)) && !defined(__STDC_NO_ATOMICS__)
// For MSVC, we need the Win32-based version, as C11 atomics
// end up preferring the actually more unlikely jump -- therefore
// triplicating the number of memory accesses.
// C11 atomics are also a MSVC 17.5 preview 2 feature (_MSC_VER >= 1935)
#  define ORC_ATOMICS_ALLOWED 1
#else
#  define ORC_ATOMICS_ALLOWED 0
#endif

#if defined(_WIN32) && !ORC_ATOMICS_ALLOWED
#include <windows.h>
typedef INIT_ONCE orc_once_atomic_int;
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && ORC_ATOMICS_ALLOWED
#include <stdatomic.h>
typedef atomic_int orc_once_atomic_int;
#else
typedef int orc_once_atomic_int;
#endif

ORC_BEGIN_DECLS

typedef struct _OrcMutex OrcMutex;

typedef struct _OrcOnce OrcOnce;

struct _OrcOnce {
  orc_once_atomic_int inited;
  void *value;
};

#  if defined(_WIN32) && !ORC_ATOMICS_ALLOWED
#define ORC_ONCE_INIT { INIT_ONCE_STATIC_INIT, NULL }
#else
#define ORC_ONCE_INIT { 0, NULL }
#endif

ORC_API void orc_once_mutex_lock (void);
ORC_API void orc_once_mutex_unlock (void);

#if defined(_WIN32) && !ORC_ATOMICS_ALLOWED
static inline orc_bool orc_once_enter(OrcOnce *once, void **value) {
  BOOL pending = FALSE;

  if (!InitOnceBeginInitialize (&once->inited, 0, &pending, NULL)) {
    DWORD err = GetLastError ();
    /* should not fail though, there's nothing we can do if it happens */
    ORC_ERROR ("InitOnceBeginInitialize failed with 0x%x", (int) err);
    return FALSE;
  }

  if (!pending) {
    *value = once->value;
    return TRUE;
  }

  return FALSE;
}

static inline void orc_once_leave(OrcOnce *once, void *value) {
  once->value = value;
  InitOnceComplete (&once->inited, 0, NULL);
}

#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && ORC_ATOMICS_ALLOWED

static inline orc_bool orc_once_enter(OrcOnce *once, void **value) {
  int inited;

  inited = atomic_load_explicit (&once->inited, memory_order_acquire);
  if (ORC_LIKELY(inited)) {
    *value = once->value;
    return TRUE;
  }

  orc_once_mutex_lock ();

  inited = atomic_load_explicit (&once->inited, memory_order_acquire);
  if (ORC_UNLIKELY(inited)) {
    *value = once->value;
    orc_once_mutex_unlock ();
    return TRUE;
  }

  return FALSE;
}

static inline void orc_once_leave(OrcOnce *once, void *value) {
  int inited = TRUE;
  once->value = value;
  atomic_store_explicit (&once->inited, inited, memory_order_release);
  orc_once_mutex_unlock ();
}

#elif ORC_GNUC_PREREQ(4, 1)

static inline orc_bool orc_once_enter(OrcOnce *once, void **value) {
  int inited;

  /* we use 0 for not initialized, 1 for initialized and 3 for currently
   * being initialized */
  inited = __sync_val_compare_and_swap(&once->inited, 0, 3);
  /* if the value was previously initialized then just return here */
  if (inited == 1) {
    *value = once->value;
    return TRUE;
  }

  orc_once_mutex_lock ();
  /* if the value was currently being initialized then check if we're the
   * thread that is doing the initialization or not */
  if (inited == 3) {
    inited = __sync_val_compare_and_swap(&once->inited, 3, 3);

    /* the other thread initialized the value in the meantime so
     * we can just return here */
    if (inited == 1) {
      *value = once->value;
      orc_once_mutex_unlock ();
      return TRUE;
    }
  }

  return FALSE;
}

static inline void orc_once_leave(OrcOnce *once, void *value) {
  once->value = value;
  /* this effectively sets the previous value of 3 to 1 */
  __sync_and_and_fetch (&once->inited, 1);
  orc_once_mutex_unlock ();
}

#else
#warning No atomic operations available

static inline orc_bool orc_once_enter(OrcOnce *once, void **value) {
  orc_once_mutex_lock ();
  if (once->inited) {
    *value = once->value;
    orc_once_mutex_unlock ();
    return TRUE;
  }

  return FALSE;
}

static inline void orc_once_leave(OrcOnce *once, void *value) {
  once->value = value;
  once->inited = TRUE;
  orc_once_mutex_unlock ();
}

#endif

ORC_END_DECLS

#endif

