#include "include.h"

// first, define the buffer pools, before the cache, which keeps per peer/thread pools isolated

struct buf_pool {
  void * buf;
  uint64_t index;
  uint64_t index_max;
  uint32_t alloc_max;
};

struct pools {
  struct buf_pool *small;
  struct buf_pool *large;
} pools;

#define ISLARGE(ptr) ((void*)ptr>=(void*)pools.large->buf)

void * slab = NULL;
void * large_buf_limit = NULL;

struct buf_pool * init_pool(uint32_t alloc_max, int32_t index_max) {
  struct buf_pool *pool = calloc(sizeof(struct buf_pool),1);
  pool->buf = slab;
  slab += index_max * alloc_max;
  pool->index_max = index_max;
  pool->alloc_max = alloc_max;
  return pool;
};

void reinit_pool(struct buf_pool * pool) {
  pool->index = 0;
};

void init_pools() {
  slab = mmap(NULL, SMALL * SMALL_MAX + LARGE * LARGE_MAX, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  assert(MAP_FAILED != slab);
  pools.small = init_pool(SMALL , SMALL_MAX);
  pools.large = init_pool(LARGE , LARGE_MAX);
  assert(pools.large->buf > pools.small->buf); // this is a given as we carve both pools from the same slab, in order.
  large_buf_limit = slab;
};

void reinit_pools() {
  reinit_pool(pools.small);
  reinit_pool(pools.large);
};

static inline void * pool_alloc(struct buf_pool *buf_pool) {
  uint_fast64_t index = atomic_fetch_add(&(buf_pool->index),1);
  assert (index < buf_pool->index_max);
  return buf_pool->buf + ( buf_pool->alloc_max * index);
};

static inline void * __pool_alloc(struct buf_pool *buf_pool) {
  struct buf_pool * rval;
  assert (buf_pool->index < buf_pool->index_max);
  // LOCKPOOL;
  rval = buf_pool->buf + ( buf_pool->alloc_max * (buf_pool->index++));
  // UNLOCKPOOL;
  return rval;
};

// below is the cache structure used to localise most buffer de/allocation

// struct defs in alloc.h
// struct cache_line { void ** free ; uint32_t free_count ; };

// struct cache {
  // struct cache_line small;
  // struct cache_line large;
// };

struct cache * init_cache() {
  return calloc(sizeof(struct cache),1); 
};

void reinit_cache(struct cache * cache) {
  memset(cache,0,sizeof(struct cache));
};

static inline void *alloc_cache_line(struct cache_line *cl) {
  void * p = NULL;
  if (cl->free) {
    p = cl->free;
    cl->free = * cl->free;
    cl->free_count--;
  };
  return p;
};

static inline void dalloc_cache_line(struct cache_line *cl, void *p) {
  if (cl->free) {
    *(void**) p = cl->free;
    cl->free = p;
  } else {
    cl->free = p;
    *(void**) p = NULL;
  };
  cl->free_count++;
};

void dalloc(struct cache * cache, void *p) {
  if (ISLARGE(p))
    dalloc_cache_line(&(cache->large),p);
  else
    dalloc_cache_line(&(cache->small),p);
};

static inline void *alloc_small(struct cache * cache) {
  void * p = alloc_cache_line(&(cache->small));
  if (NULL==p)
    p = pool_alloc(pools.small);
  assert (p);
  return p;
};

static inline void *alloc_large(struct cache * cache) {
  void * p = alloc_cache_line(&(cache->large));
  if (NULL==p)
    p = pool_alloc(pools.large);
  assert (p);
  return p;
};

void *alloc(struct cache * cache, size_t sz) {
  assert(LARGE >= sz);
  if (SMALL < sz)
    return alloc_large(cache);
  else
    return alloc_small(cache);
};

void init_alloc() {
   init_pools();
   // gcache = init_cache();
};

void reinit_alloc() {
   reinit_pools();
   // reinit_cache(gcache);
};

void report_route_table () {
  printf("route table size = unreliable\n");
  // printf("route table size = %d (%d/%d)\n", small_index + large_index, small_index, large_index);
};
