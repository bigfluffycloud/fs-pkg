/*
 * tk/servers/fs-pkg:
 *	Package filesystem. Allows mounting various package files as
 *   a read-only (optionally with spillover file) overlay filesystem
 *   via FUSE or (eventually) LD_PRELOAD library.
 *
 * Copyright (C) 2012-2018 BigFluffy.Cloud <joseph@bigfluffy.cloud>
 *
 * Distributed under a MIT license. Send bugs/patches by email or
 * on github - https://github.com/bigfluffycloud/fs-pkg/
 *
 * No warranty of any kind. Good luck!
 */
/* i18n.c: Internationalization support using dconf */
#include "conf.h"
#include "dict.h"
#include "i18n.h"
#include "logger.h"

dict *i18n_load(const char *path) {
     dict *rv = dconf_load(path);

     if (rv == NULL) {
        Log(LOG_FATAL, "Failed to load i18n data: %s", path);
        return NULL;
     }

     return rv;
}

int i18n_init(void) {
    Log(LOG_INFO, "Loading internationalization (i18n) support...");
    return 0;
}
