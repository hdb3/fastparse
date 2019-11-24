#ifndef __PEER_H
#define __PEER_H
struct peer {
  uint32_t static_local_pref;
  bool  ebgp;
  uint32_t bgpid;
  uint32_t peer_address;
  struct route** adj_rib_in;
  // below the line is internal stuff which shoul logically be unlinked from the application level
  pthread_t thread_id;
  void *base;
  int64_t length;
  struct cache *cache;
};

extern int npeers;
extern struct peer peers[];
void init_peers ();
void reinit_peers ();
#endif
