
#include "include.h"

void phase1(struct peer * peer, struct route **r){
  (*r) -> tiebreak.ebgp = peer->ebgp;
  (*r) -> tiebreak.peer_address = peer->peer_address;
  (*r) -> tiebreak.local_pref = peer->static_local_pref;
  (*r) -> tiebreak.bgpid = peer->bgpid;
};
