#ifndef __PEERGROUP_H
#define __PEERGROUP_H
extern inline struct route * serialize_ibgp(struct route *route);
struct peergroup {
  struct route * (*serialize)(struct route *route);
  int fd;
};
extern inline void phase3(struct peergroup *peergroup, struct route *route, uint32_t *addrreftable, uint32_t table_index);
extern int npeergroups;
extern struct peergroup peergroups[];
void init_peergroups ();
#endif
