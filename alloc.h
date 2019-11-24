#ifndef __ALLOC_H
#define __ALLOC_H

#define LARGE (4096 + 128)
// LARGE_MAX : number of large buffers in the system
#define LARGE_MAX 1000000LL

#define SMALL (256)
// SMALL_MAX: number of small buffers in the system
#define SMALL_MAX 40000000LL

struct cache_line { void ** free ; uint32_t free_count ; };

struct cache {
  struct cache_line small;
  struct cache_line large;
};

struct cache * init_cache();
void reinit_cache(struct cache * cache);
void init_alloc();
void reinit_alloc();
extern inline void dalloc(struct cache * cache, void *p);
extern inline void *alloc(struct cache * cache, size_t sz);
void report_route_table ();
#endif
