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
 *
 * src/warden.c:
 *	Warden manages a bunch of jailfs based jails
 */
#include <lsd/lsd.h>
#include <stdio.h>
int main(int argc, char **argv) {
    printf("Warden is not yet ready, but hopefully will exist by 1.1 release!\n");
}