#include "include.h"

// from kakapo/core/util.c
// (modified)
uint32_t toHostAddress(char *s) {
  struct in_addr addr;
  assert(0 != inet_aton(s, &addr));
  return __bswap_32(addr.s_addr);
};
// end from kakapo/core/util.c

static inline void *alloc_adj_rib_in() {
  return calloc(BIG, sizeof(void*));
};

static inline void zero_adj_rib_in(void*p) {
  memset(p,0,BIG*sizeof(void*));
};

int npeers = 0;
struct peer peers[10];

void reinit_peers () {
  int pn;
  for (pn=0;pn<npeers;pn++) {
    zero_adj_rib_in(peers[pn].adj_rib_in);
    ; // zero out the rib is the main thing.....
  };
};

void init_peers () {
  // peers[npeers++] = (struct peer){ static_local_pref, ebgp, bgpid, peer_address, adj_rib_in};
  peers[npeers++] = (struct peer){ 42, true, toHostAddress("192.168.1.1"), toHostAddress("192.168.1.1"), (struct route**) alloc_adj_rib_in()};
  peers[npeers++] = (struct peer){ 43, false, toHostAddress("192.168.2.1"), toHostAddress("192.168.2.1"), (struct route**) alloc_adj_rib_in()};
  peers[npeers++] = (struct peer){ 44, false, toHostAddress("192.168.3.1"), toHostAddress("192.168.3.1"), (struct route**) alloc_adj_rib_in()};
  // peers[npeers++] = (struct peer){ 43, false, toHostAddress("192.168.2.1"),toHostAddress("192.168.2.1")};
};
