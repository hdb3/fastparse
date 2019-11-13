#ifndef __ALLOC_H
#define __ALLOC_H

#define LARGE (4096 + 128)
// LARGE_MAX is 1M
#define LARGE_MAX 1000000LL

#define SMALL (256)
// SMALL_MAX is 10M
#define SMALL_MAX 40000000LL

extern void * small_buf = NULL;
extern void * large_buf = NULL;
extern void * large_buf_limit = NULL;

extern void ** small_free = NULL;
extern void ** large_free = NULL;

extern uint32_t small_index = 0;
extern uint32_t large_index = 0;

extern uint32_t small_free_count = 0;
extern uint32_t large_free_count = 0;


extern void * both_buf;

void init_alloc();
void reinit_alloc();
static inline void dalloc(void *p);
static inline void *alloc(size_t sz);
void report_route_table ();
#endif
