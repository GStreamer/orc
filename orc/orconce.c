

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <orc/orconce.h>
#include <orc/orcdebug.h>

#if defined(HAVE_THREAD_PTHREAD)

#include <pthread.h>

static pthread_mutex_t once_mutex = PTHREAD_MUTEX_INITIALIZER;

void
_orc_once_init (void)
{
  
}

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

#elif defined(HAVE_THREAD_WIN32)

#include <windows.h>

static CRITICAL_SECTION once_mutex;

void
_orc_once_init (void)
{
  InitializeCriticalSection (&once_mutex);
}

void
orc_once_mutex_lock (void)
{
  EnterCriticalSection (&once_mutex);
}

void
orc_once_mutex_unlock (void)
{
  LeaveCriticalSection (&once_mutex);
}

#else

void
_orc_once_init (void)
{
}

void
orc_once_mutex_lock (void)
{
}

void
orc_once_mutex_unlock (void)
{
}

#endif


