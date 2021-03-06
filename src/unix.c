/*
 * tk/servers/jailfs:
 *	Package filesystem. Allows mounting various package files as
 *   a read-only (optionally with spillover file) overlay filesystem
 *   via FUSE or (eventually) LD_PRELOAD library.
 *
 * Copyright (C) 2012-2019 BigFluffy.Cloud <joseph@bigfluffy.cloud>
 *
 * Distributed under a MIT license. Send bugs/patches by email or
 * on github - https://github.com/bigfluffycloud/jailfs/
 *
 * No warranty of any kind. Good luck!
 */
#include <sys/resource.h>
#include <sys/wait.h>
#include <lsd/lsd.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "shell.h"
#include "unix.h"
#include "conf.h"
#include "cron.h"
#include "debugger.h"
#include "module.h"
int g_argc = -1;
char **g_argv = NULL;

/* src/module.c */
extern int module_dying(int signal);
/* src/conf.c */
extern void conf_reload(void);
static char pidfile_name[PATH_MAX];
static FILE *pidfile = NULL;

/*
 * Signal handling
 *	SIGHUP:	Reload configuration and flush caches/sync databases
 *	SIGUSR1: Dump profiling data
 *	SIGUSR2: Toggle profiling
 *	SIGTERM: Sync databases and gracefully terminate
 *      SIGSEGV: Restart module or daemon instead of crashing, dumping core if supported
 *	SIGKILL: Get into a consistent state and shut down
 *
 * Special signals
 *	SIGILL: Illegal instruction- debugger
 *	SIGTRAP: Trap into debugger
 */

static void signal_handler(int signal) {
   Log(LOG_WARNING, "Caught signal %d", signal);

   if (signal == SIGTERM || signal == SIGQUIT || signal == SIGKILL)
      conf.dying = 1;

   if (signal == SIGSEGV) {
      // Emit a hopefully helpful stack dump
      stack_unwind();
#if	defined(CONFIG_MODULES)
      if (in_module) {
         if (module_dying(signal) != 0)
            daemon_restart();
      } else
#endif
         conf.dying = 1;
#if	defined(CONFIG_PROFILING)
   } else if (signal == SIGUSR1) {
      profiling_dump();
   } else if (signal == SIGUSR2) {
      profiling_toggle();
#endif
   } else if (signal == SIGHUP) {
      conf_reload();
   } else if (signal == SIGCHLD) { /* Prevent zombies */
      while(waitpid(-1, NULL, WNOHANG) > 0)
         ;
   } else if (signal == SIGTERM || signal == SIGKILL) {
      goodbye();
   }
}

struct SignalMapping {
   int signum;		/* Signal number */
   int flags;		/* See man 3 sigaction */
   int blocked;		/* Signals blocked during handling */
};

static struct SignalMapping SignalSet[] = {
   { SIGHUP, SA_RESTART, 0 },
   { SIGUSR1, SA_RESTART, SIGUSR1|SIGHUP },
   { SIGUSR2, SA_RESTART, SIGUSR2|SIGHUP },
   { SIGQUIT, SA_RESETHAND, SIGUSR1|SIGUSR2|SIGHUP },
   { SIGTERM, SA_RESTART, SIGUSR1|SIGUSR2|SIGHUP|SIGTERM },
   { SIGSEGV, SA_RESETHAND|SA_ONSTACK, SIGHUP|SIGSEGV },
   { SIGCHLD, SA_RESTART, SIGCHLD },
   { SIGILL, SA_RESTART, SIGILL|SIGSEGV|SIGTRAP },
   { SIGTRAP, SA_RESTART, SIGILL|SIGSEGV|SIGTRAP },
   { SIGPIPE, SA_RESTART, SIGPIPE|SIGCHLD },
   { -1, -1, -1 },
};
static int signal_set[] = { SIGHUP, SIGUSR1, SIGUSR2, SIGQUIT, SIGTERM, SIGPIPE, SIGCHLD, SIGSEGV, SIGCHLD };

static void signal_init(void) {
   struct sigaction action, old;
   int i = 0;

   sigfillset(&action.sa_mask);
   action.sa_handler = signal_handler;

#define	setsz(x)	(sizeof(x) / sizeof(x[0]))
   do {
     if (signal_set[i]) {
        /* If SIGSEGV, switch to debugger stack */
        if (signal_set[i] == SIGSEGV)
           action.sa_flags = SA_ONSTACK|SA_RESETHAND;
        else
           action.sa_flags = 0;

        if (sigaction(signal_set[i], &action, &old) == -1)
           Log(LOG_ERR, "sigaction: signum:%d [%d] %s", i, errno, strerror(errno));
     }
     i++;
   } while (i < (sizeof(signal_set) / sizeof(signal_set[0])));
}

int daemon_restart(void) {
   if (g_argc != -1 && g_argv != NULL) {
      (void)execv(g_argv[0], g_argv);
      Log(LOG_ERR, "Couldn't restart server: (%d) %s", errno, strerror(errno));
   }
   exit(-1);
   return -1;		// not reached
}

static void posix_signal_quiet(void) {
   sigset_t sigset;
   int signals[] = { SIGABRT, SIGQUIT, SIGKILL, SIGTERM, SIGSYS };
   int sigign[] = { SIGCHLD, SIGPIPE, SIGHUP, SIGSTOP, SIGTSTP, SIGCONT };

   /* these should be handled by debugger: SIGTRAP, SIGFPE, SIGSEGV */
   sigemptyset(&sigset);

   for (int i = 0; i <= (size_t)(sizeof(signals) / sizeof(signals[0])); i++)
       sigaddset(&sigset, signals[i]);

   pthread_sigmask(SIG_SETMASK, &sigset, NULL);

   for (int i = 0; i <= (size_t)(sizeof(sigign) / sizeof(sigign[0])); i++)
       signal(sigign[i], SIG_IGN);
}

static void enable_coredump(void) {
   struct rlimit rl;
   rl.rlim_max = RLIM_INFINITY;
   setrlimit(RLIMIT_CORE, &rl);
}

static void unlimit_fds(void) {
   struct rlimit rl;
   rl.rlim_max = RLIM_INFINITY;
   setrlimit(RLIMIT_NOFILE, &rl);
}

void host_init(void) {
   enable_coredump();
   unlimit_fds();
   signal_init();
}

void pidfile_close(void) {
   // close the file descriptor
   // XXX: Should we do this in pidfile_open? I prefer to keep lockfiles open until exit()
   // XXX: unless we're hurting for fds
   fclose(pidfile);
   unlink(pidfile_name);
   Log(LOG_INFO, "Unlinked pidfile %s", pidfile_name);
   memset(pidfile_name, 0, sizeof(pidfile_name));
}

int	pidfile_open(const char *path) {
   pid_t pid = 0;

   if (pidfile_name[0] != (char)0) {
      Log(LOG_DEBUG, "it seems you've called pidfile_open() more than once...");
      return -1;
   }

   if (file_exists(path)) {
      errno = EADDRINUSE;
      return -1;
   }

   if ((pidfile = fopen(path, "w")) == NULL) {
      Log(LOG_EMERG, "pidfile_open: failed to open pid file %s: %d (%s)", path, errno, strerror(errno));
   }

   pid = getpid();
   fprintf(pidfile, "%i\n", pid);
   fflush(pidfile);
   // Store the pid file name so we can unlink it on exit
   memset(pidfile_name, 0, sizeof(pidfile_name));
   memcpy(pidfile_name, path, sizeof(pidfile_name));
   Log(LOG_INFO, "Wrote pid file %s: %d", pidfile_name, pid);
   return 0;
}
