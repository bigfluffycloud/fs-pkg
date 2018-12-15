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
 * src/vfs.h:
 *	Virtual File System support
 */
#if	!defined(__VFS_H)
#define	__VFS_H
#define	FUSE_USE_VERSION 26
#include <fuse/fuse.h>
#include <fuse/fuse_lowlevel.h>
#include <fuse/fuse_opt.h>

struct vfs_handle {
//   char        pkg_file[PATH_MAX];     /* package path */
//   off_t       pkg_offset;             /* offset within package */
   int	       pkgid;	      /* key used to find the mmap pointer */
   u_int32_t   fileid;        /* file ID inside the package XXX: should this be inode #?? */
   char        file[PATH_MAX];         /* file name */
   size_t      len;                    /* length of file */
//   off_t       offset;                 /* current offset in file */
   char       *maddr;                  /* mmap()'d region of package */
};

struct vfs_watch {
   u_int32_t   mask;
   int         fd;
   char        path[PATH_MAX];
};

struct vfs_fake_stat {
   u_int32_t   st_ino;
   mode_t      st_mode;
   off_t       st_size;
   uid_t       st_uid;
   gid_t       st_gid;
   time_t      st_time;
   char        path[PATH_MAX];
};

struct CacheItem {
   int dirty;
   dict *cache;
};
typedef struct CacheItem CacheItem_t;

typedef struct vfs_handle vfs_handle_t;
typedef struct vfs_watch vfs_watch_t;

extern int  vfs_dir_walk(void);

extern BlockHeap *vfs_handle_heap;
extern BlockHeap *vfs_inode_heap;
extern BlockHeap *vfs_watch_heap;

extern u_int32_t vfs_root_inode;
extern dlink_list vfs_watch_list;

extern void vfs_fuse_fini(void);
extern void vfs_fuse_init(void);

extern void vfs_op_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);
extern void vfs_op_access(fuse_req_t req, fuse_ino_t ino, int mask);
extern void vfs_op_readlink(fuse_req_t req, fuse_ino_t ino);
extern void vfs_op_opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);
extern void vfs_op_readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi);
extern void vfs_op_releasedir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);
extern void vfs_op_open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);
extern void vfs_op_release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);
extern void vfs_op_read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi);
extern void vfs_op_statfs(fuse_req_t req, fuse_ino_t ino);
extern void vfs_op_getxattr(fuse_req_t req, fuse_ino_t ino, const char *name, size_t size);
extern void vfs_op_listxattr(fuse_req_t req, fuse_ino_t ino, size_t size);
extern void vfs_op_lookup(fuse_req_t req, fuse_ino_t parent, const char *name);
extern void vfs_op_setattr(fuse_req_t req, fuse_ino_t ino, struct stat *attr, int to_set, struct fuse_file_info *fi);
extern void vfs_op_mknod(fuse_req_t req, fuse_ino_t ino, const char *name, mode_t mode, dev_t rdev);
extern void vfs_op_mkdir(fuse_req_t req, fuse_ino_t ino, const char *name, mode_t mode);
extern void vfs_op_symlink(fuse_req_t req, const char *link, fuse_ino_t parent, const char *name);
extern void vfs_op_unlink(fuse_req_t req, fuse_ino_t ino, const char *name);
extern void vfs_op_rmdir(fuse_req_t req, fuse_ino_t ino, const char *namee);
extern void vfs_op_rename(fuse_req_t req, fuse_ino_t parent, const char *name, fuse_ino_t newparent, const char *newname);
extern void vfs_op_link(fuse_req_t req, fuse_ino_t ino, fuse_ino_t newparent, const char *newname);
extern void vfs_op_write(fuse_req_t req, fuse_ino_t ino, const char *buf, size_t size, off_t off, struct fuse_file_info *fi);
extern void vfs_op_setxattr(fuse_req_t req, fuse_ino_t ino, const char *name, const char *value, size_t size, int flags);
extern void vfs_op_removexattr(fuse_req_t req, fuse_ino_t ino, const char *name);
extern void vfs_op_create(fuse_req_t req, fuse_ino_t ino, const char *name, mode_t mode, struct fuse_file_info *fi);

extern void *thread_vfs_init(void *data);
extern void *thread_vfs_fini(void *data);

extern int  vfs_watch_init(void);
extern void vfs_watch_fini(void);
extern vfs_watch_t *vfs_watch_add(const char *path);
extern int  vfs_watch_remove(vfs_watch_t * watch);
struct pkg_inode {
   u_int32_t   st_ino;
   mode_t      st_mode;
   off_t       st_size;
   off_t       st_off;
   uid_t       st_uid;
   gid_t       st_gid;
   time_t      st_time;
};

typedef struct pkg_inode pkg_inode_t;

extern void vfs_inode_init(void);
extern void vfs_inode_fini(void);

#endif	// !defined(__VFS_H)
