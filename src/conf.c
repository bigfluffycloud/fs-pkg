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
/*
 * Dictionary based configuration lookup
 * 	Used for single-entry configurations
 *	See lconf.[ch] for list-based configuration
 *	suitable for ACLs and similar options.
 *
 * Copyright (C) 2008-2018 bigfluffy.cloud
 *
 * This code wouldn't be possible without N. Devillard's dictionary.[ch]
 * from the iniparser package. Thanks!!
 */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <strings.h>
#include "conf.h"
#include "dict.h"
#include "logger.h"
#include "str.h"

void dconf_init(const char *file) {
   FILE       *fp;
   char       *p;
   char        buf[512];
   int         in_comment = 0;
   int         line = 0;
   char        *discard = NULL;
   char       *key, *val;
   _DCONF_DICT = dict_new();

   if (!(fp = fopen(file, "r"))) {
      Log(LOG_FATAL, "unable to open config file %s", file);
      raise(SIGTERM);
   }

   while (!feof(fp)) {
      line++;
      memset(buf, 0, 512);
      discard = fgets(buf, sizeof(buf), fp);

      /*
       * did we get a line? 
       */
      if (buf == NULL)
         continue;

      p = buf;

      str_strip(p);
      p = str_whitespace_skip(p);

      /*
       * did we eat the whole line? 
       */
      if (strlen(p) == 0)
         continue;

      /*
       * fairly flexible comment handling
       */
      if (p[0] == '*' && p[1] == '/') {
         in_comment = 0;               /* end of block comment */
         continue;
      } else if (p[0] == ';' || p[0] == '#' || (p[0] == '/' && p[1] == '/')) {
         continue;                     /* line comment */
      } else if (p[0] == '/' && p[1] == '*')
         in_comment = 1;               /* start of block comment */

      if (in_comment)
         continue;                     /* ignored line, in block comment */

      key = p;

      if ((val = strchr(p, '=')) != NULL) {
         *val = '\0';
         val++;
      }

      if (val == NULL)
         continue;

      val = str_unquote(val);
      dconf_set(key, val);
   }
}

void dconf_fini(void) {
   dict_mem_free(_DCONF_DICT);
   _DCONF_DICT = NULL;
}

int dconf_get_bool(const char *key, const int def) {
   char       *tmp;
   int         rv = 0;

   if ((tmp = dconf_get_str(key, NULL)) == NULL)
      return def;
   else if (strcasecmp(tmp, "true") == 0 || strcasecmp(tmp, "on") == 0 ||
            strcasecmp(tmp, "yes") == 0 || (int)strtol(tmp, NULL, 0) == 1)
      rv = 1;
   else if (strcasecmp(tmp, "false") == 0 || strcasecmp(tmp, "off") == 0 ||
            strcasecmp(tmp, "no") == 0 || (int)strtol(tmp, NULL, 0) == 0)
      rv = 0;

   return rv;
}

double dconf_get_double(const char *key, const double def) {
   char       *tmp;

   if ((tmp = dconf_get_str(key, NULL)) == NULL)
      return def;

   return atof(tmp);
}

int dconf_get_int(const char *key, const int def) {
   char       *tmp;

   if ((tmp = dconf_get_str(key, NULL)) == NULL)
      return def;

   return (int)strtol(tmp, NULL, 0);
}

char       *dconf_get_str(const char *key, const char *def) {
   if (_DCONF_DICT == NULL || key == NULL)
      return NULL;

   return dict_get(_DCONF_DICT, key, def);
}

time_t dconf_get_time(const char *key, const time_t def) {
   return (timestr_to_time(dconf_get_str(key, NULL), def));
}

int dconf_set(const char *key, const char *val) {
   return dict_add(_DCONF_DICT, key, val);
}

void dconf_unset(const char *key) {
   dict_del(_DCONF_DICT, key);
}
