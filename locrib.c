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

void locrib(uint32_t extended_address, struct route *new) {

  uint32_t address = _LR_INDEX_MASK & extended_address;
  void* extended_current = LOCRIB[address];
  struct route * current = CLEAR64(extended_current);

  bool eob_flag = _LR_EOB & extended_address;
  bool push_flag = ISSET64(extended_current);

  if ( ( NULL == current )  || // the RIB was empty for this address
                               // so the tiebreak is not needed....
      (tiebreaker(&new->tiebreak,&current->tiebreak))) {
    LOCRIB[address] = SET64(new);
    if (!(push_flag))
      locribj_push(extended_address);
  };

  if (eob_flag)
    schedule_phase3();  // this is the point at which input processing for this route can stop
                        //  and start work on other update messages
};

void locrib_withdraw(uint32_t extended_address, struct route *new) {

  uint32_t address = _LR_INDEX_MASK & extended_address;
  void* extended_current = LOCRIB[address];
  struct route * current = CLEAR64(extended_current);

  bool eob_flag = _LR_EOB & extended_address;
  bool push_flag = ISSET64(extended_current);

  if ( NULL == CLEAR64(current ))    // the RIB was empty for this address
                                     // which should not happen for a withdraw....
    assert(NULL != CLEAR64(current ));

  else if (current != new) ; // the withdraw is not for the current winner
                             // the route should still be removed, but no
			     // route push is needed
  else                     { // the withdraw is for the current winner
                             // TODO
			     // this will trigger full adj-rib-in re-selection
			     // but for now i just treat as withdraw (stateless)

    LOCRIB[address] = SET64(NULL);

    if (!(push_flag))
      locribj_push(_LR_NULL_ROUTE | extended_address);

    if (eob_flag)
      schedule_phase3();  // this is the point at which input processing for this route can stop
                          //  and start work on other update messages
  };
};
