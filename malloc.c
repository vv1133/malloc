/*
 * libc/stdlib/malloc/malloc.c -- malloc function
 *
 *  Copyright (C) 2002,03  NEC Electronics Corporation
 *  Copyright (C) 2002,03  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 * 
 * Written by Miles Bader <miles@gnu.org>
 */

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>

#include "malloc.h"
#include "heap.h"


/* The malloc heap.  We provide a bit of initial static space so that
   programs can do a little mallocing without mmaping in more space.  */
HEAP_DECLARE_STATIC_FREE_AREA (initial_fa, 256);
struct heap __malloc_heap = HEAP_INIT_WITH_FA (initial_fa);

static void *
malloc_from_heap (size_t size, struct heap *heap)
{
  void *mem;

  MALLOC_DEBUG (1, "malloc: %d bytes", size);

  /* Include extra space to record the size of the allocated block.  */
  size += MALLOC_HEADER_SIZE;

  __heap_lock (heap);

  /* First try to get memory that's already in our heap.  */
  mem = __heap_alloc (heap, &size);

  __heap_unlock (heap);

  if (likely (mem))
    /* Record the size of the block and get the user address.  */
    {
      mem = MALLOC_SETUP (mem, size);

      MALLOC_DEBUG (-1, "malloc: returning 0x%lx (base:0x%lx, total_size:%ld)",
		    (long)mem, (long)MALLOC_BASE(mem), (long)MALLOC_SIZE(mem));
    }
  else
    MALLOC_DEBUG (-1, "malloc: returning 0");

  return mem;
}

static void *
malloc_add_block (void *block, size_t block_size, struct heap *heap)
{
    /*
	 * We manage the block memory from the argument, add it to the heap.
     */
    if (likely (block != (void *)-1))
	{
	  MALLOC_DEBUG (1, "adding system memroy to heap: 0x%lx - 0x%lx (%d bytes)",
			(long)block, (long)block + block_size, block_size);

	  /* Get back the heap lock.  */
	  __heap_lock (heap);

	  /* Put BLOCK into the heap.  */
	  __heap_free (heap, block, block_size);

	  MALLOC_DEBUG_INDENT (-1);

	  __heap_unlock (heap);
	}
	return block;
}

#if 0
#define MAX_MALLOC_BLOCK_SIZE (16 * 1024 * 1024)
#define MIN_MALLOC_BLOCK_SIZE (4096)

static void
malloc_add_mem (void *base, size_t size, size_t bsize)
{
	int    i;
	void  *block;
	struct heap *heap = &__malloc_heap;
	void *pbase, *pend, *abase, *aend;

	if (size <= 0 || bsize < MIN_MALLOC_BLOCK_SIZE)
	{
		return;
	}

	if ((size / bsize) >= 2)
	{
		for (i = size / (2 * bsize); i < size / bsize; i++)
		{
			block = (long)base + ((long)(i * bsize) & (long)bsize);
			malloc_add_block (block, bsize, heap);
			abase = block + bsize;
		}

		pbase = base;
		pend = (long)base + 
					((long)(bsize * (size / (2 * bsize))) & (long)bsize);
		if (pend > pbase)
		{
			malloc_add_mem (pbase, pend - pbase, bsize / 2);
		}
	
		aend = base + size;
		abase = base;
		malloc_add_mem (abase, aend - abase, bsize / 2);
	}
	malloc_add_mem (base, size, bsize / 2);
}
#endif

void
malloc_init (void *base, size_t size)
{
	//malloc_add_mem (base, size, MAX_MALLOC_BLOCK_SIZE);
	malloc_add_block (base, size, &__malloc_heap);
}

void *
malloc (size_t size)
{
  void *mem;
#ifdef MALLOC_DEBUGGING
  static int debugging_initialized = 0;
  if (! debugging_initialized)
    {
      debugging_initialized = 1;
      __malloc_debug_init ();
    }
  if (__malloc_check)
    __heap_check (&__malloc_heap, "malloc");
#endif

  if (unlikely (size == 0))
    size++;

  /* Check if they are doing something dumb like malloc(-1) */
  if (unlikely(((unsigned long)size > (unsigned long)(MALLOC_HEADER_SIZE*-2))))
    goto oom;

  mem = malloc_from_heap (size, &__malloc_heap);
  if (unlikely (!mem))
    {
    oom:
      return 0;
    }

  return mem;
}
