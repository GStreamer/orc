

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <orc/orconce.h>
#include <pthread.h>


pthread_mutex_t once_mutex = PTHREAD_MUTEX_INITIALIZER;


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

