This is a pretty old version of jailfs. I'm working to restore it to a
usable state. Unfortunately the newest version is on a drive that will not
spin up... So far it runs, can load packages into the database, and almost
do basic vfs ops again!

I wrote this some years ago, abandoned it for a few years, then was asked
to resurrect it for a project at $FormerEmployer. Unfortunately, they
had some really horrible ideas for what it was to become. Needless to say,
this is a clean-room reimplementation of a lot of the code, with a focus on
stability and security. The jailfs shipped with $Product appears to have
grown to be quite a mess in the years since I left :O 


There's a bit of duplicate code here as I work to replace
some of the older code here with newer code from other projects.

In particular, the ThreadPool code is much improved, adding
the ability to better isolate modules and allow respawning
damaged threads without restarting the entire process.

I hope to soon get a chance to clean up the VFS code and make
this more modular.

The long term plan for the project is to provide a threaded,
modular FUSE based virtual file system, allowing stacking
file systems (fs-etc (template based /etc) on top of jailfs).

pkg (archive), spillover (.spill), and etc are needed for
release 0.1.0

Eventually adding support for archive format modules
(libarchive, etc) and what not via the module system.
-----------------------

# Install libev xar and fuse.
* apt install xar libxar-dev libev-dev fuse-dev libxml2-dev libarchive-dev

# Build stuff
* make clean world

# Configure it
* joe jailfs.cf

# Run it
* ./jailfs &

# Build a test package
* make testpkg

# Confirm that inotify, etc work ---

[2018/11/23 01:04:57]      info: importing pkg pkg/irssi.pkg
[2018/11/23 01:04:57]      info: package pkg/irssi.pkg seems valid, committing...


# You should see files in your mountpoint (path.mountpoint) if everything is working

# Bugs
There's plenty of them. This is REALLY old code... a version before it was
"ruined" to meet the needs of a small dirty hack of a project at
$FormerEmployer.

If you can fix them, submit a diff -u or git diff to me at
joseph@bigfluffy.cloud

If you can at least send a crash report (edit jailfs.cf and set the below):

  log.level=debug
  log.stack_unwind=true

Then send your log file (default is jailfs.log) by email.

Debug log level WILL reduce performance. stack_unwind should not have a
noticable effect on normal operation.
