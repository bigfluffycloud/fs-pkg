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
 * Copyright (C) 2005 William Pitcock, et al.
 * Rights to this code are as documented in doc/LICENSE.
 *
 * Data structures for the block allocator.
 *
 * $Id: balloc.h 7779 2007-03-03 13:55:42Z pippijn $
 */

#ifndef __BALLOC
#define __BALLOC
#include <sys/types.h>
#include <stdlib.h>
#include "dlink.h"
struct Block {
   size_t      alloc_size;
   struct Block *next;                 /* Next in our chain of blocks */
   void       *elems;                  /* Points to allocated memory */
   dlink_list  free_list;
   dlink_list  used_list;
};
typedef struct Block Block;

struct MemBlock {
#ifdef DEBUG_BALLOC
   unsigned long magic;
#endif
   dlink_node  self;
   Block      *block;                  /* Which block we belong to */
};

typedef struct MemBlock MemBlock;

/* information for the root node of the heap */
struct BlockHeap {
   dlink_node  hlist;
   char        name[64];               /* heap name */
   size_t      elemSize;               /* Size of each element to be stored */
   unsigned long elemsPerBlock;        /* Number of elements per block */
   unsigned long blocksAllocated;      /* Number of blocks allocated */
   unsigned long freeElems;            /* Number of free elements */
   Block      *base;                   /* Pointer to first block */
};
typedef struct BlockHeap BlockHeap;

extern int  blockheap_free(BlockHeap * bh, void *ptr);
extern void *blockheap_alloc(BlockHeap * bh);

extern BlockHeap *blockheap_create(size_t elemsize, int elemsperblock, const char *name);
extern int  blockheap_destroy(BlockHeap * bh);

extern void blockheap_init(void);
extern void blockheap_usage(BlockHeap * bh, size_t * bused, size_t * bfree, size_t * bmemusage);

#endif                                 /* !defined(__BALLOC) */