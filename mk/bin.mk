ifeq (${CONFIG_TOC_LIBXML2}, y)
CFLAGS += -DCONFIG_TOC_LIBXML2
endif
CFLAGS += -DCONFIG_VFS_FUSE

bin := fs-pkg
objs += .obj/atomicio.o
objs += .obj/balloc.o
objs += .obj/conf.o
objs += .obj/db_sqlite.o
objs += .obj/dictionary.o
objs += .obj/dlink.o
objs += .obj/evt.o
objs += .obj/inode.o
objs += .obj/logger.o
objs += .obj/main.o
objs += .obj/pkg.o
objs += .obj/pkg_map.o
objs += .obj/pkg_toc.o
objs += .obj/profiling.o
objs += .obj/signal_handler.o
objs += .obj/str.o
objs += .obj/support.o
objs += .obj/thread_pool.o
objs += .obj/timestr.o
objs += .obj/vfs.o
objs += .obj/vfs_fuse.o
objs += .obj/vfs_inotify.o
objs += .obj/vfs_pkg.o
objs += .obj/vfs_spill.o
${bin}: ${objs}
	@${CC} -o $@ ${LDFLAGS} ${extra_libs} $^
ifeq (${CONFIG_STRIP_BINS}, y)
	@echo "[STRIP] $@"
	@strip $@
endif

.obj/%.o:src/%.c
	@echo "[CC] $< => $@"
	@${CC} ${warn_flags} ${CFLAGS} -o $@ -c $<
