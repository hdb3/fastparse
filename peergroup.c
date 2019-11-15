#include "include.h"

int npeergroups = 1;
struct peergroup peergroups[1];
void init_peergroups () {
  FILE* file = fopen("ibgp.bin", "w" );
  peergroups[0] = (struct peergroup){ serialize_ibgp , file };
};
