#ifndef __PEER_H
#define __PEER_H
struct peer {
  uint32_t static_local_pref;
  bool  ebgp;
  uint32_t bgpid;
  uint32_t peer_address;
  struct route** adj_rib_in;
};

extern int npeers;
extern struct peer peers[];
void init_peers ();
void reinit_peers ();
#endif
