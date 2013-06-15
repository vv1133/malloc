/*
 * libc/stdlib/malloc/free.c -- free function
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
#include <sys/mman.h>

#include "malloc.h"
#include "heap.h"


static void
free_to_heap (void *mem, struct heap *heap)
{
  size_t size;
  struct heap_free_area *fa;

  /* Check for special cases.  */
  if (unlikely (! mem))
    return;

  /* Normal free.  */

  MALLOC_DEBUG (1, "free: 0x%lx (base = 0x%lx, total_size = %d)",
		(long)mem, (long)MALLOC_BASE (mem), MALLOC_SIZE (mem));

  size = MALLOC_SIZE (mem);
  mem = MALLOC_BASE (mem);

  __heap_lock (heap);

  /* Put MEM back in the heap, and get the free-area it was placed in.  */
  fa = __heap_free (heap, mem, size);

  /* Nope, nothing left to do, just release the lock.  */
  __heap_unlock (heap);

  MALLOC_DEBUG_INDENT (-1);
}

void
free (void *mem)
{
  free_to_heap (mem, &__malloc_heap);
}
