#include "include.h"

extern void serialize_ebgp (struct route *route, uint8_t ** q_base , uint16_t q_max );
extern void serialize_ibgp (struct route *route, uint8_t ** q_base , uint16_t q_max );
extern void serialize_copy (struct route *route, uint8_t ** q_base , uint16_t q_max );

int npeergroups = 0;
struct peergroup peergroups[10];

void reinit_peergroups () {
  int pn;
  for (pn=0;pn<npeergroups;pn++) {
    struct peergroup *pgp = &peergroups[pn];
    if (pgp->file)
      rewind(pgp->file);
  };
};

void init_peergroups () {

  // NULL FILE* allows testing of serilaisation without file IO overhead

  // peergroups[npeergroups++] = (struct peergroup){ serialize_ibgp , NULL };
  // peergroups[npeergroups++] = (struct peergroup){ serialize_ebgp , NULL };
  // peergroups[npeergroups++] = (struct peergroup){ serialize_ibgp , fopen("/dev/null", "w" ) };
  // peergroups[npeergroups++] = (struct peergroup){ serialize_ebgp , fopen("/dev/null", "w" ) };
  peergroups[npeergroups++] = (struct peergroup){ serialize_ibgp , fopen("ibgp.bin", "w" ) };
  peergroups[npeergroups++] = (struct peergroup){ serialize_ebgp , fopen("ebgp.bin", "w" ) };
  // peergroups[npeergroups++] = (struct peergroup){ serialize_copy , fopen("copy.bin", "w" ) };
};
