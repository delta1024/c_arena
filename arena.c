#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arena.h"
#define ARENA_BLOCK_SIZE 2048
struct arena_block_tag {
	ptrdiff_t block_size;
};
#define arena_get_tag_ptr(ptr) &((struct arena_block_tag*)(ptr))[-1]
arena_alloc arena_init() {
	arena_alloc a = {0};
	a.beg = malloc(ARENA_BLOCK_SIZE);
	a.end = a.beg ? a.beg+ARENA_BLOCK_SIZE : 0;
	return a;
}
void *arena_raw_malloc(arena_alloc *a, ptrdiff_t size, ptrdiff_t align, ptrdiff_t count) {
	ptrdiff_t padding = -(uintptr_t)a->beg & (align - 1);
	// If alignment is bigger than padding size we store the tag in the padding
	if (padding < sizeof(struct arena_block_tag)) {
		// make room for the tag
		a->beg += sizeof(struct arena_block_tag);
		padding = -(uintptr_t)a->beg & (align - 1);

	}
	ptrdiff_t avalible = a->end - a->beg - padding;
	if (avalible < 0 || count > avalible/size) {
		abort();
	}
	void *p = a->beg + padding;
	struct arena_block_tag *tag = arena_get_tag_ptr(p);
	tag->block_size = count*size;
	a->beg += padding + count*size;
	return memset(p, 0, count*size);
}
void arena_free(arena_alloc *a) {
	ptrdiff_t avalible = a->end - a->beg;
	ptrdiff_t offset = ARENA_BLOCK_SIZE - avalible;
	void *malloc_ptr = a->beg - offset;
	free(malloc_ptr);
}
int main() {
	arena_alloc a = arena_init();
	char *buff = arena_malloc(&a, char, strlen("hello") + 1 );
	memcpy(buff, "hello", strlen("hello"));
	printf("%s\n", buff);
	struct arena_block_tag *tag = arena_get_tag_ptr(buff);
	assert(tag->block_size == (strlen("hello") + 1) * sizeof(char));
	arena_free(&a);
} 
