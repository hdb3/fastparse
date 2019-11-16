
#include "include.h"
#define __BODY__ serialize_ebgp
#include "bgpserialize.tpl"
#undef __BODY__

  get(ORIGIN);
  set(1,0x40,ORIGIN,attr_ptr);
  get(AS_PATH);
  prepend(42);
  get(NEXT_HOP);
  copy();
  // copy();
  // TODO
  // if the attributes are wrongly ordered then the macine silenetly fails
  // by copying the previous attribute.....!!!!
  get(COMMUNITY);
  copy();

  *q_base = q;
};
