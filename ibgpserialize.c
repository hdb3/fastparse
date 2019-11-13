
#include "include.h"
#undef __TRAILER__
#define __BODY__ serialize_ibgp
#include "bgpserialize.tpl"
#undef __BODY__

  get(ORIGIN);
  set(1,0x40,ORIGIN,attr_ptr);
  get(AS_PATH);
  copy();
  get(NEXT_HOP);
  copy();
  set4(0x40,LOCAL_PREF,route->tiebreak.local_pref);

#define __TRAILER__
#include "bgpserialize.tpl"
#undef __TRAILER__
