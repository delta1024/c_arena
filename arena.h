/* arena.h: Interface to an arena allocator
 * Copyright 2024 Jacob Stannix
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the “Software”), to deal 
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
 * copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all 
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
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
