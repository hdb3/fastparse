#include "include.h"

/*
 * this is the unoptimised version of locrib hich demonstrates performance for the
 * simple replacement cases
 *
 * for complete operation it needs to be enhanced to support search of peer ARIs when a current winner is replaced or withdrawn
 *
*/

struct locrib_entry *LOCRIB=NULL;

void locrib_init() {

  int i; 
  assert(NULL==LOCRIB);
  LOCRIB = calloc(BIG, sizeof(struct locrib_entry));
  for (i=0;i<BIG;i++)
    pthread_spin_init(&LOCRIB[i].spinlock,PTHREAD_PROCESS_PRIVATE);
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

void locrib(uint32_t extended_addrref, struct route *new, bool update) {

  // functional overview
  //
  // input is addrref (table index) and pointer to a new peer route
  // (not sure why the 'withdraw' aspect cannot be signalled with 'NULL', but there was a reason i think....) 
  //
  // the possible effects are
  // 1) change in locrib 'head' ( a new winner)
  // 2) push of this addref into the journal
  // 3) phase 3 schedule trigger
  //
  // conditionalities:
  // 1) locrib head change is straight forward tiebreak ((implied) withdraw slightly different)
  // 2) journal push depends on
  //   a) locrib head change
  //   b) no previous push flag (this is a locrib_entry field, possibly still a high bit set on the route, but better an atomic bool)
  // 3) phase 3 schedule trigger depends on all of the above AND the eob (end-of-block flag) set from the adj_rib_in process

  uint32_t addrref = _LR_INDEX_MASK & extended_addrref;
  bool eob_flag = _LR_EOB & extended_addrref;
  bool push_flag;
  struct locrib_entry * lre = &LOCRIB[addrref];

  inline void push() {
    push_flag = lre->push_flag;
    lre->push_flag = true;
    lre->head = new;
  };

  pthread_spin_lock(&lre->spinlock);

  do {
    struct route * current = lre->head;
    if (update) {

#ifndef NEWWINS
      if ( NULL == current || tiebreaker(&new->tiebreak,&current->tiebreak))
#else
      if (1) // this is the 'new always wins tiebreaker - produces maximum churn'
#endif
        push();
  // process withdraw
    } else {
       if ( current && current == new) // there is an entry in the table AND it matches this route
          push(NULL);
    };
  } while (0);

  pthread_spin_unlock(&lre->spinlock);

  if (!(push_flag)) {
    locribj_push(addrref);
    if (eob_flag)
      schedule_phase3(0);
  };
};
