

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <orc/orconce.h>

#if defined(HAVE_THREAD_PTHREAD)

#include <pthread.h>

static pthread_mutex_t once_mutex = PTHREAD_MUTEX_INITIALIZER;

void
orc_once_mutex_lock (void)
{
  pthread_mutex_lock (&once_mutex);
}

void
orc_mutex_unlock (OrcMutex *mutex)
{
  pthread_mutex_unlock (&once_mutex);
}

#elif defined(HAVE_THREAD_WIN32)

#include <windows.h>

static CRITICAL_SECTION once_mutex;

void
orc_once_mutex_lock (void)
{
  EnterCriticalSection (&once_mutex);
}

void
orc_mutex_unlock (OrcMutex *mutex)
{
  LeaveCriticalSection (&once_mutex);
}

#else

void
orc_once_mutex_lock (void)
{
}

void
orc_mutex_unlock (OrcMutex *mutex)
{
}

#endif


