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
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "conf.h"
#include "logger.h"

FILE       *log_open(const char *path) {
   FILE       *fp;
   char       *lvl;

   lvl = dconf_get_str("log.level", 0);

   /*
    * parse the debug level 
    */
   if (!strcasecmp(lvl, "debug"))
      conf.log_level = LOG_DEBUG;
   else if (!strcasecmp(lvl, "info"))
      conf.log_level = LOG_INFO;
   else if (!strcasecmp(lvl, "hack"))
      conf.log_level = LOG_HACK;
   else if (!strcasecmp(lvl, "warning"))
      conf.log_level = LOG_WARNING;
   else if (!strcasecmp(lvl, "error"))
      conf.log_level = LOG_ERROR;
   else if (!strcasecmp(lvl, "fatal"))
      conf.log_level = LOG_FATAL;

   if ((fp = fopen(path, "w")) == NULL) {
      Log(LOG_ERROR, "Unable to open log file %s: %d (%s)", path, errno, strerror(errno));
      return NULL;
   }

   return fp;
}

void log_close(FILE * fp) {
   if (fp != NULL) {
      fclose(fp);
      fp = NULL;
   }
}

void Log(enum log_priority priority, const char *fmt, ...) {
   va_list     ap;
   char        timestamp[64];
   time_t      t;
   struct tm  *tm;
   char       *level = NULL;
   char	      buf[1024];

   if (priority < conf.log_level) {
      return;
   }

   if (!conf.log_fp)
      conf.log_fp = stderr;

   va_start(ap, fmt);
   t = time(NULL);
   tm = localtime(&t);

   switch (priority) {
      case LOG_DEBUG:
         level = "debug";
         break;
      case LOG_INFO:
         level = "info";
         break;
      case LOG_WARNING:
         level = "warn";
         break;
      case LOG_ERROR:
         level = "error";
         break;
      case LOG_FATAL:
         level = "fatal";
         break;
      case LOG_HACK:
         level = "hack";
         break;
      default:
         level = "???";
         break;
   }

   memset(buf, 0, sizeof(buf));
   strftime(timestamp, sizeof(timestamp) - 1, "%Y/%m/%d %H:%M:%S", tm);
   vsprintf(buf, fmt, ap);

   if ((conf.log_fp != stderr) && dconf_get_bool("sys.daemonize", 0) == 0)
      printf("%s %5s: %s\n", timestamp, level, buf);

   fprintf(conf.log_fp, "%s %5s: %s\n", timestamp, level, buf);
   fflush(conf.log_fp);

   va_end(ap);
}
