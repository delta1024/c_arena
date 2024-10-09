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
struct arena_block {
	char *beg;
	char *end;
	struct arena_block *next;
};
static arena_block *arena_block_init() {
	arena_block *a = malloc(sizeof(arena_block) + ARENA_BLOCK_SIZE);
	if (a == NULL)
		return NULL;
	a->beg = (char *)&a[1];
	a->end = a->beg + ARENA_BLOCK_SIZE;
	a->next = NULL;
	return a;
}

arena_alloc arena_init() {
	arena_alloc a = {0};
	a.blocks = arena_block_init();
	return a;
}
void *arena_raw_malloc(arena_alloc *a, ptrdiff_t size, ptrdiff_t align, ptrdiff_t count) {
	ptrdiff_t padding = -(uintptr_t)a->blocks->beg & (align - 1);
	// If alignment is bigger than padding size we store the tag in the padding
	if (padding < sizeof(struct arena_block_tag)) {
		// make room for the tag
		a->blocks->beg += sizeof(struct arena_block_tag);
		padding = -(uintptr_t)a->blocks->beg & (align - 1);

	}
	ptrdiff_t avalible = a->blocks->end - a->blocks->beg - padding;
	if (avalible < 0 || count > avalible/size) {
		arena_block *block = arena_block_init();
		if (block == NULL){
			abort();
		}
		block->next = a->blocks;
		a->blocks = block;
		return arena_raw_malloc(a, size, align, count);
	}
	void *p = a->blocks->beg + padding;
	struct arena_block_tag *tag = arena_get_tag_ptr(p);
	tag->block_size = count*size;
	a->blocks->beg += padding + count*size;
	return memset(p, 0, count*size);
}
void arena_free(arena_alloc *a) {
	arena_block *block = a->blocks;
	while (block != NULL) {
		a->blocks = block->next;
		free(block);
		block = a->blocks;
	}
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
