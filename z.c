#include "stdio.h"

#define CHUNKSIZE(L) ((L + 7) / 8)

int main (int argc, char** argv) {

  int i;

  for (i=0; i<33 ; i++) {
    printf("i: %d CHUNKSIZE(i) %d\n",i,CHUNKSIZE(i));
  };
};
