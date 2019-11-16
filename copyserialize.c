
#include "include.h"
#define __BODY__ serialize_copy
#include "bgpserialize.tpl"
#undef __BODY__

  get(ORIGIN);
  copy();
  get(AS_PATH);
  copy();
  get(NEXT_HOP);
  copy();
  // TODO
  // if the attributes are wrongly ordered then the macine silenetly fails
  // by copying the previous attribute.....!!!!
  get(COMMUNITY);
  copy();

  *q_base = q;
};
