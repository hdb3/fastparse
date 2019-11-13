#ifndef __ALLOC_H
#define __ALLOC_H

#define LARGE (4096 + 128)
// LARGE_MAX is 1M
#define LARGE_MAX 1000000LL

#define SMALL (256)
// SMALL_MAX is 10M
#define SMALL_MAX 40000000LL

extern void * small_buf;
extern void * large_buf;
extern void * large_buf_limit;

extern void ** small_free;
extern void ** large_free;

extern uint32_t small_index;
extern uint32_t large_index;

extern uint32_t small_free_count;
extern uint32_t large_free_count;

extern void * both_buf;

void init_alloc();
void reinit_alloc();
void dalloc(void *p);
void *alloc(size_t sz);
void report_route_table ();
#endif
