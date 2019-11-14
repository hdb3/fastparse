#include "include.h"

int npeergroups = 1;
struct peergroup peergroups[1];
void init_peergroups () {
  // int fd = open("/dev/null",O_WRONLY);
  int fd = open("ibgp.bin",O_WRONLY | O_CREAT, 0666 );
  assert (-1 != fd);
  peergroups[0] = (struct peergroup){ serialize_ibgp , fd };
};
