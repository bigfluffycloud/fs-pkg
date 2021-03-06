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
#if	!defined(__PKG_H)
#define	__PKG_H

struct pkg_handle {
   int         pkgid;		       /* database pkgid */
   int         fd;                     /* file descriptor for mmap() */
   int         refcnt;                 /* references to this package */
   time_t      otime;                  /* open time (for garbage collector) */
   char       *name;                   /* package name */
   u_int32_t   id;                     /* package ID */
   void	      *addr;		       /* mmap return address */
};

struct pkg_object {
   u_int32_t   pkg;
   char       *name;
   char        type;
   uid_t       owner;
   gid_t       grp;
   size_t      size;
   off_t       offset;
   time_t      ctime;
   u_int32_t   inode;
};

struct pkg_file_mapping {
   char       *pkg;                    /* package file */
   size_t      len;                    /* length of subfile */
   off_t       offset;                 /* offset of subfile */
   int         fd;                     /* file descriptor for mmap */
   void       *addr;                   /* mmap return address */
};

/* Initialize package handling/caching subsystem */
extern void pkg_init(void);


/* Release our instance of package */
extern void pkg_close(struct pkg_handle *pkg);

//
extern char *pkg_extract_file(u_int32_t pkgid, const char *path);

// Stuff for mmap()ing files from packages - XXX: BROKEN!
extern void pkg_unmap_file(struct pkg_file_mapping *p);
extern struct pkg_file_mapping *pkg_map_file(const char *path, size_t len, off_t offset);
extern struct pkg_handle *pkg_handle_byname(const char *path);

/* Open a package */
extern struct pkg_handle *pkg_open(const char *path);
/* 'import' a package, called by vfs watcher */
extern int  pkg_import(const char *path);

/* 'forget' a package, called by vfs_watcher */
extern int  pkg_forget(const char *path);

// garbage collect
extern int pkg_gc(void);

#endif	// !defined(__PKG_H)
