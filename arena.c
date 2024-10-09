#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arena.h"
#define ARENA_BLOCK_SIZE 2048
typedef struct arena_block_tag {
	char *beg, *end;
	struct arena_block_tag *next;
} arena_block_tag;
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
static char *arena_block_check_tag_space(char *beg, char *end, ptrdiff_t align);
static ptrdiff_t arena_block_get_padding_size(char *beg, char *end, ptrdiff_t align);
static ptrdiff_t arena_block_region_avalible(char *beg, char *end, ptrdiff_t padding) {
	ptrdiff_t avalible = end - beg - padding;
	return avalible;
}
static ptrdiff_t arena_block_get_padding_size(char *beg, char *end, ptrdiff_t align){
	ptrdiff_t padding = -(uintptr_t)beg & (align - 1);
	char *my_beg = beg;
	// If alignment is bigger than padding size we store the tag in the padding
	if (padding < sizeof(struct arena_block_tag)) {
		// make room for the tag
		my_beg += sizeof(struct arena_block_tag);
		padding = -(uintptr_t)my_beg & (align - 1);
	}
	return padding;
}

static char *arena_block_check_tag_space(char *beg, char *end, ptrdiff_t align) {
	ptrdiff_t padding = -(uintptr_t)beg & (align - 1);
	char *my_beg = beg;
	// If alignment is bigger than padding size we store the tag in the padding
	if (padding < sizeof(struct arena_block_tag)) {
		// make room for the tag
		my_beg += sizeof(struct arena_block_tag);
	}
	return my_beg;
}
static void *arena_raw_block_malloc(arena_alloc *a, ptrdiff_t size, ptrdiff_t align, ptrdiff_t count) {
	ptrdiff_t padding = arena_block_get_padding_size(a->blocks->beg, a->blocks->end, align);
	char *beg = arena_block_check_tag_space(a->blocks->beg, a->blocks->end, align);
	ptrdiff_t avalible = arena_block_region_avalible(beg, a->blocks->end, padding);
	if (avalible < 0 || count > avalible/size) {
		arena_block *block = arena_block_init();
		if (block == NULL){
			abort();
		}
		block->next = a->blocks;
		a->blocks = block;
		return arena_raw_block_malloc(a, size, align, count);
	}
	void *p = beg + padding;
	struct arena_block_tag *tag = arena_get_tag_ptr(p);
	tag->beg = a->blocks->beg;
	tag->end = p + count*size;
	tag->next = NULL;
	a->blocks->beg = tag->end;
	return memset(p, 0, count*size);
}
void *arena_raw_malloc(arena_alloc *a, ptrdiff_t size, ptrdiff_t align, ptrdiff_t count) {
	arena_block_tag *prev, *tag = NULL;
	char *beg;
	ptrdiff_t avalible, padding ;
	prev = a->free_list;
	tag = prev;
	while (tag != NULL) {
		padding = arena_block_get_padding_size(tag->beg, tag->end, align);
		beg = arena_block_check_tag_space(tag->beg, tag->end, align);
		avalible = arena_block_region_avalible(beg, tag->end, padding);
		if (avalible < 0 || count > avalible/size) {
			prev = tag;
			tag = tag->next;
			continue;
		} else {
			prev->next = tag->next;
		}
		break;
	}
	if (tag == NULL) {
		return arena_raw_block_malloc(a, size, align, count);
	}
	char *true_beg = tag->beg;
	char *true_end = tag->end;

	void *p = beg + padding;
	struct arena_block_tag *new_tag = arena_get_tag_ptr(p);
	new_tag->beg = true_beg;
	new_tag->end = true_end;
	new_tag->next = NULL;
	return memset(p, 0, count*size);
}
void arena_free(arena_alloc *a, void *ptr) {
	arena_block_tag *tag = arena_get_tag_ptr(ptr);
	tag->next = a->free_list;
	a->free_list = tag;
}
void arena_deinit(arena_alloc *a) {
	arena_block *block = a->blocks;
	while (block != NULL) {
		a->blocks = block->next;
		free(block);
		block = a->blocks;
	}
}
int main() {
	arena_alloc a = arena_init();

	size_t *buff = arena_malloc(&a, size_t, strlen("hello") + 1 );
	struct arena_block_tag *tag = arena_get_tag_ptr(buff);
	arena_free(&a, buff);
	char *buff2 = arena_malloc(&a, char, strlen("hi") + 1);
	struct arena_block_tag *tag2 = arena_get_tag_ptr(buff);
	assert(tag->beg == tag2->beg);
	assert(tag->end == tag2->end);
	arena_deinit(&a);
} 
