/*
 * tk/servers/fs-pkg:
 *	Package filesystem. Allows mounting various package files as
 *   a read-only (optionally with spillover file) overlay filesystem
 *   via FUSE or (eventually) LD_PRELOAD library.
 *
 * Copyright (C) 2012-2018 BigFluffy.Cloud <joseph@bigfluffy.cloud>
 *
 * Distributed under a BSD license. Send bugs/patches by email or
 * on github - https://github.com/bigfluffycloud/fs-pkg/
 *
 * No warranty of any kind. Good luck!
 */
#define	VERSION "0.0.2"
#include <sys/mount.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define	FUSE_USE_VERSION 26
#include <fuse/fuse.h>
#include <fuse/fuse_opt.h>
#include <fuse/fuse_lowlevel.h>
#include "conf.h"
#include "db.h"
#include "evt.h"
#include "inode.h"
#include "logger.h"
#include "pkg.h"
#include "signal_handler.h"
#include "thread_pool.h"
#include "vfs.h"
#include "vfs_inotify.h"

struct conf conf;

void goodbye(void) {
   Log(LOG_INFO, "shutting down...");
   dconf_fini();
   vfs_watch_fini();
   vfs_fuse_fini();
   umount(conf.mountpoint);
   inode_fini();
   dlink_fini();
   log_close(conf.log_fp);
   exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
   int         fd;
   struct fuse_args margs = FUSE_ARGS_INIT(0, NULL);
   conf.born = conf.now = time(NULL);
   umask(0);

   atexit(goodbye);

   signal_init();
   evt_init();
   blockheap_init();
   dconf_init("fs-pkg.cf");
   dlink_init();
   pkg_init();
   inode_init();

   if (dconf_get_bool("sys.daemonize", 0) == 1) {
      fprintf(stdout, "going bg, bye!\n");
      /*
       * XXX: add daemon crud 
       */
   }

   /*
    * open log file, if its valid, otherwise assume debug mode and use stdout 
    */
   if ((conf.log_fp = log_open(dconf_get_str("path.log", NULL))) == NULL)
      conf.log_fp = stdout;

   Log(LOG_INFO, "fs-pkg: Package filesystem %s starting up...", VERSION);
   Log(LOG_INFO, "Copyright (C) 2012-2018 bigfluffy.cloud -- See LICENSE in distribution package for terms of use");

   if (conf.log_level == LOG_DEBUG)
      Log(LOG_WARNING, "Log level is set to DEBUG - this will hurt performance badly. Use log.level=info if not debugging...");

   if (!conf.mountpoint)
      conf.mountpoint = dconf_get_str("path.mountpoint", "chroot/");

   /*
    * only way to make gcc happy...argh;) -bk 
    */
   vfs_fuse_args = margs;

   /*
    * The fuse_mount() options get modified, so we always rebuild it 
    */
   if ((fuse_opt_add_arg(&vfs_fuse_args, argv[0]) == -1 ||
        fuse_opt_add_arg(&vfs_fuse_args, "-o") == -1 ||
        fuse_opt_add_arg(&vfs_fuse_args, "nonempty,allow_other") == -1))
      Log(LOG_ERROR, "Failed to set FUSE options.\n");

   umount(conf.mountpoint);
   vfs_fuse_init();
   Log(LOG_INFO, "Opening database %s", dconf_get_str("path.db", ":memory"));
   db_sqlite_open(dconf_get_str("path.db", ":memory"));

   /*
    * set up the watch subsystem 
    */
   vfs_watch_init();

   /*
    * walk the package dirs and import all existing packages 
    */
   vfs_dir_walk();

   /*
    * the big event loop 
    */
   while (!conf.dying) {
      ev_loop(evt_loop, 0);
   }

   /*
    * shouldnt be reached... 
    */
   return EXIT_SUCCESS;
}
