

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <orc/orconce.h>
#include <orc/orcdebug.h>

#if defined(HAVE_THREAD_PTHREAD)

#include <pthread.h>

static pthread_mutex_t once_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;

void
orc_once_mutex_lock (void)
{
  pthread_mutex_lock (&once_mutex);
}

void
orc_once_mutex_unlock (void)
{
  pthread_mutex_unlock (&once_mutex);
}

void
orc_global_mutex_lock (void)
{
  pthread_mutex_lock (&global_mutex);
}

void
orc_global_mutex_unlock (void)
{
  pthread_mutex_unlock (&global_mutex);
}

#elif defined(HAVE_THREAD_WIN32)

#include <windows.h>

static SRWLOCK once_mutex = SRWLOCK_INIT;
static SRWLOCK global_mutex = SRWLOCK_INIT;

void
orc_once_mutex_lock (void)
{
  AcquireSRWLockExclusive (&once_mutex);
}

void
orc_once_mutex_unlock (void)
{
  ReleaseSRWLockExclusive (&once_mutex);
}

void
orc_global_mutex_lock (void)
{
  AcquireSRWLockExclusive (&global_mutex);
}

void
orc_global_mutex_unlock (void)
{
  ReleaseSRWLockExclusive (&global_mutex);
}

#else

void
orc_once_mutex_lock (void)
{
}

void
orc_once_mutex_unlock (void)
{
}

#endif


