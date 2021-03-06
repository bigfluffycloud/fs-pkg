/*
 * tk/servers/jailfs:
 *	Package filesystem. Allows mounting various package files as
 *   a read-only (optionally with spillover file) overlay filesystem
 *   via FUSE or (eventually) LD_PRELOAD library.
 *
 * Copyright (C) 2012-2018 BigFluffy.Cloud <joseph@bigfluffy.cloud>
 *
 * Distributed under a MIT license. Send bugs/patches by email or
 * on github - https://github.com/bigfluffycloud/jailfs/
 *
 * No warranty of any kind. Good luck!
 */
/* Thread pools */
#include <lsd/lsd.h>
#include "shell.h"
#include "threads.h"
#include "unix.h"
#include "conf.h"
int core_ready = 0;
pthread_mutex_t core_ready_m;
pthread_cond_t core_ready_c;

ThreadPool *threadpool_init(const char *name, const char *opts) {
  ThreadPool *r;
  int tmp;

  if (!(r = (ThreadPool *)mem_alloc(sizeof(ThreadPool)))) {
     Log(LOG_EMERG, "%s: allocation failed: [%d] %s", __FUNCTION__, errno, strerror(errno));
     return NULL;
  }

  if (pthread_attr_init(&(r->pth_attr)) != 0) {
     Log(LOG_EMERG, "%s: failed: %s (%d)", __FUNCTION__, strerror(errno), errno);
     return NULL;
  }

  if ((tmp = dict_getInt(conf.dict, "tuning.threads.stacksz", 0)) > 0) {
     if (pthread_attr_setstacksize(&(r->pth_attr), tmp) != 0)
        Log(LOG_ERR, "%s: Unable to set stack size: %s (%d)", __FUNCTION__, strerror(errno), errno);
  }

  r->name = strdup(name);
  r->list = create_list();

  return r;
}

int threadpool_destroy(ThreadPool *pool) {
    if (pool == NULL)
       return -1;

    if (pool->list != NULL) {
       destroy_list(pool->list);
       pool->list = NULL;
    }

    if (pool->name != NULL) {
       mem_free(pool->name);
       pool->name = NULL;
    }
    mem_free(pool);

    return 0;
}

Thread *thread_create(ThreadPool *pool, void *(*init)(void *), void *(*fini)(void *), void *arg, const char *descr) {
  Thread *tmp = NULL;
  char thrname[16];
  int rv = -1;

  tmp = mem_alloc(sizeof(Thread));
  memset(tmp, 0, sizeof(Thread));
  memcpy((void *)&tmp->thr_attr, &(pool->pth_attr), sizeof(tmp->thr_attr));

  if (pthread_create(&tmp->thr_info, &tmp->thr_attr, init, &arg) != 0) {
     Log(LOG_ERR, "thread (%s) creation failed: %s (%d)", descr, strerror(errno), errno);
     return NULL;
  }

  // update refcnt
  tmp->refcnt++;

  /* Set pointer to destructor (if given) */
  if (fini)
     tmp->fini = fini;

  /* Add to thread pool */
  list_add(pool->list, tmp, sizeof(tmp));

  if (dconf_get_bool("debug.threads", 0) == 1)
     Log(LOG_DEBUG, "new thread %x (%s) created in pool %s", tmp, descr, pool->name);

  // Set up the thread name (for ps/htop/etc)
  memset(thrname, 0, sizeof(thrname));
  snprintf(thrname, sizeof(thrname) - 1 /* leave trailing NULL */, "%s", descr);

  if ((rv = pthread_setname_np(tmp->thr_info, thrname)) != 0)
     Log(LOG_SHELL, "Failed setting thread name: rv=%d (%d) %s", rv, errno, strerror(errno));

  return tmp;
}

Thread *thread_shutdown(ThreadPool *pool, Thread *thr) {
   if (!pool || !thr)
      return NULL;

   thr->refcnt--;

   /* If reference count hits 0, destroy the thread */
   /* Theory: This should reduce some cache churn by keeping
    *   the fairly heavyweight worker threads from being restarted
    *   so often.
    * ToDo: This should do more error checking
    * Side-cases:
    */
   if (thr->refcnt <= 0) {
      if (dconf_get_bool("debug.threads", 0) == 1) {
         Log(LOG_DEBUG, "unallocating thread %x due to refcnt == 0", thr);

         if (thr->refcnt < 0)
            Log(LOG_DEBUG, "BUG: thread %x refcnt is negative: %d", thr->refcnt);
      }

      if (thr->argv)
         mem_free(thr->argv);

      list_remove(pool->list, FRONT);
      mem_free(thr);
      return NULL;
   }

   return thr;
}

/*
 * You *MUST* call these at the beginning and end of your threads or bad things will happen
 */
void thread_entry(dict *_conf) {
    /* block until main thread is read */
    pthread_mutex_lock(&core_ready_m);

    while (core_ready < 1)
       pthread_cond_wait(&core_ready_c, &core_ready_m);

    pthread_mutex_unlock(&core_ready_m);
    host_init();
}

void thread_exit(dict *_conf) {
    pthread_exit(NULL);
}
