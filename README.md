This shit is a bit of mess right now...
Keep tracking the git, it'll clean up fairly quickly. Feel free to send patches!

We are trying to make things as modular as possible without depending on dlfcn...
Patches to: joseph@bigfluffy.cloud

--- If anyone wants an example of using fuse with libev,
--- take a look in vfs_fuse.c:vfs_fuse_init and
--- vfs_fuse.c:vfs_fuse_read_cb

* We use a slightly trimmed down libfuse, however the original lib
* should work just fine- if you dont mind the extra bloat
-- This is needed because a lot of systems complain that
-- libfuse is out of date using the native libfuse.

----------
* We are working libbombsquad- MIT licensed stuff for interacting with
  with libfuse. What a pain in the ass...
