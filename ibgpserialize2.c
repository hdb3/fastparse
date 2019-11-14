
#include "include.h"
#define __BODY__ serialize_ibgp
#include "bgpserialize2.tpl"
#undef __BODY__

  get(ORIGIN);
  set(1,0x40,ORIGIN,attr_ptr);
  get(AS_PATH);
  copy();
  get(NEXT_HOP);
  copy();
  // TODO
  // if the attributes are wrongly ordered then the macine silenetly fails
  // by copying the previous attribute.....!!!!
  get(COMMUNITY);
  copy();
  set4(0x40,LOCAL_PREF,route->tiebreak.local_pref);

  *q_base = q;
};
