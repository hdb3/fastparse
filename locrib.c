#include "include.h"

/*
 * this is the unoptimised version of locrib hich demonstrates performance for the
 * simple replacement cases
 *
 * for complete operation it needs to be enhanced to support search of peer ARIs when a current winner is replaced or withdrawn
 *
*/

struct route **LOCRIB=NULL;

void locrib_init() {
  LOCRIB = calloc(BIG, sizeof(void*));
  locribj_init();
};


/*
 * there are some critical scheduling issues in this function:
 * 1) pushing an action request in to the ARO queue - but,
 *    this is only done when there is not already an action request - 
 *    this prevents multiple updates for the same route being queued,
 *    and also provides a hard guard over the amount of queued work and corresponding storage
 * 2) the locrib is the main point of concurrency control if multiple threads are working on input updates
 *    concurrent;y.
 *    Simple logic would dictate a spin lock over the locrib entry.
 * 3) Pushing work onto the ARO queue is also the trigger to start 
 *    working the queue, if it is not already active.  THis can be done after/outside the spin lock...
 *
 *  The deatil of push logic follows:
 *    locrib is only updated if the tiebreak favours the new route
 *    updating locrib ALWAYS sets the pushed bit (it is only cleared in ARO processing)
 *    updating locrib ONLY pushes a ARO action item when it finds that the push bit has been claered
 *    This ensures that ARO processing will come back to this route promptly.
 *
 * The stored state (per address) is the pushed bit and another bit for the spin lock
*/

void locrib(uint32_t extended_address, struct route *new, bool update) {

  uint32_t address = _LR_INDEX_MASK & extended_address;
  void* extended_current = LOCRIB[address];
  struct route * current = CLEAR64(extended_current);

  bool eob_flag = _LR_EOB & extended_address;
  bool push_flag = ISSET64(extended_current);

  inline void push(struct route *route) {
    LOCRIB[address] = SET64(route);
    if (!(push_flag)) {
      locribj_push(address);
      if (eob_flag)
        schedule_phase3(0);
    };
  };

  if (update) {

#ifndef NEWWINS
    if ( NULL == current || tiebreaker(&new->tiebreak,&current->tiebreak))
#else
    if (1) // this is the 'new always wins tiebreaker - produces maximum churn'
#endif
      push(new);


  // process withdraw
  } else {
     if ( current && current == new) // there is an entry to consider and it is this route
        push(NULL);
  };
};
