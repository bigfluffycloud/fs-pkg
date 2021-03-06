# mk/bin.mk:
#	Rules for building host binaries
#
# Copyright (C) 2018 Bigfluffy.cloud <joseph@bigfluffy.cloud>
#
# Distributed under a MIT license. Send bugs/patches by email or
# on github - https://github.com/bigfluffycloud/jailfs/
#
# No warranty of any kind. Good luck!
#

bin/jailfs: ${jailfs_objs} lib/libsd.a
	@echo "[LD] ($^) => $@"
	${CC} -o $@ $^ ${libs} ${extra_libs} ${LDFLAGS}
ifeq (${CONFIG_STRIP_BINS}, y)
	@echo "[STRIP] $@"
	@strip $@
endif

warden:
	@echo "* Skipping warden as it isn't going to be ready until 1.1 :("

.obj/%.o:src/%.c
	@echo "[CC] $< => $@"
	${CC} ${warn_flags} ${CFLAGS} -o $@ -c $<

.obj/kilo.o:src/kilo.c
	@echo "[CC] [NOWARN] $< => $@"
	${CC} ${warn_noerror} ${CFLAGS} -o $@ -c $<

###########################
# XXX: This needs removed #
###########################
.obj/vfs.o:src/vfs.c
	@echo "[CC] [NOWARN] $< => $@"
	${CC} ${warn_noerror} ${CFLAGS} -o $@ -c $<

####################
