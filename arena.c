#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ARENA_BLOCK_SIZE 2048
typedef struct arena_alloc {
	char *beg;
	char *end;
} arena_alloc;
arena_alloc arena_init() {
	arena_alloc a = {0};
	a.beg = malloc(ARENA_BLOCK_SIZE);
	a.end = a.beg ? a.beg+ARENA_BLOCK_SIZE : 0;
	return a;
}
void *arena_raw_malloc(arena_alloc *a, ptrdiff_t size, ptrdiff_t align, ptrdiff_t count) {
	ptrdiff_t padding = -(uintptr_t)a->beg & (align - 1);
	ptrdiff_t avalible = a->end - a->beg - padding;
	if (avalible < 0 || count > avalible/size) {
		abort();
	}
	void *p = a->beg + padding;
	a->beg += padding + count*size;
	return memset(p, 0, count*size);
}
void arena_free(arena_alloc *a) {
	ptrdiff_t avalible = a->end - a->beg;
	ptrdiff_t offset = ARENA_BLOCK_SIZE - avalible;
	void *malloc_ptr = a->beg - offset;
	free(malloc_ptr);
}
#define arena_malloc(a, t, n) (t *)arena_raw_malloc(a, sizeof(t), alignof(t), n)
int main() {
	arena_alloc a = arena_init();
	char *buff = arena_malloc(&a, char, strlen("hello") + 1 );
	memcpy(buff, "hello", strlen("hello"));
	printf("%s\n", buff);
	arena_free(&a);
} 
