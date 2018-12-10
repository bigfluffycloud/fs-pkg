#
# mk/debug.mk:
#	Debugging stuff
#
# Copyright (C) 2018 Bigfluffy.cloud <joseph@bigfluffy.cloud>
#
# Distributed under a MIT license. Send bugs/patches by email or
# on github - https://github.com/bigfluffycloud/jailfs/
#
# No warranty of any kind. Good luck!
#

dbg/${bin}.symtab:
	nm -Clp ${bin} | \
	awk '{ printf "%s %s %s\n", $$3, $$2, $$4 }' | \
	egrep -v "(@@|__FUNCTION)" | \
	sort -u > dbg/${bin}.symtab

dbg/${bin}.strings:
	strings ${bin} \
	> dbg/${bin}.strings

debug_targets += dbg/${bin}.symtab
debug_targets += dbg/${bin}.strings
debug_clean += ${debug_targets}

debug: debug-pre ${debug_targets} ${extra_debug_targets} debug-post

debug-pre:
	@echo "Building debugging data..."

debug-post:
	@echo "Finished building debugging data!"

extra_target += debug
extra_clean += ${debug_clean}
