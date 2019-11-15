#ifndef __PEERGROUP_H
#define __PEERGROUP_H
extern inline void serialize_ibgp (struct route *route, uint8_t ** q_base , uint16_t q_max );
extern inline void serialize_copy (struct route *route, uint8_t ** q_base , uint16_t q_max );
struct peergroup {
  void (*serialize)(struct route *route, uint8_t ** q_base , uint16_t q_max );
  FILE* file;
};
extern int npeergroups;
extern struct peergroup peergroups[];
void init_peergroups ();
#endif
