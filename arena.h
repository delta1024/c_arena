#ifndef _ARENA_ALLOCATOR_H__
#define _ARENA_ALLOCATOR_H__
#include <stdalign.h>
#include <stddef.h>
typedef struct arena_block arena_block;
typedef struct arena_block_tag arena_block_tag;
typedef struct arena_alloc {
	arena_block *blocks;
	arena_block_tag *free_list;
} arena_alloc;
arena_alloc arena_init();
void *arena_raw_malloc(arena_alloc *a, ptrdiff_t size, ptrdiff_t align, ptrdiff_t count);
void arena_free(arena_alloc *a, void *ptr);
void arena_deinit(arena_alloc *a);
#define arena_malloc(a, t, n) (t *)arena_raw_malloc(a, sizeof(t), alignof(t), n)
#endif // _ARENA_ALLOCATOR_H__
