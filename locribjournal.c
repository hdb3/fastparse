#include "include.h"

/* locrib journal
 *
 * log locrib changes unti they are actioned by the output chain
 * a simple wrap-around buffer of address refernces
 * the producer writes an entry and increments the write cursor
 * the consumer, when the read cursor is behind the write cursor,
 * reads values from the current read cursor and increments it
 * the cursors wrap around when the reach the buffer limit
 * the buffer should be at most the allowed max size of the RIB
 * logic elsewhere guarantees that duplicate entries can not be pushed
 * the consumer re-enables updates for an address as it reads the current locrib.
 * The atomic operation is required for the locrib update only....
 *
 *
*/

// concurrency considerations
//
// sepcifically addressing single consumer, multiple producers
// (not implying that this is less general though...)
//
// the shared structures are the cursor and the cursor content
//
// by first (atomically) incrementing the cursor the cursor content update can be
// safely protected from other producers, however a consumer could still read the
// incomplete stream
// A trailing cursor could help with this, however that presents problems for multiple
// producers, albeit implausible if the control flow is invariant.
// The strategy is increment the forward pointer; update the journal entry, increment the trailing pointer
// The only issue would be if one producer ran faster than another, in which case the increments would allow access to the 
// 'wrong' content.  This is VERY implausible.  A detection protection would be to write on invalid value into empty journal entries
// which would allow an 'early'read to be detected.
// A failsafe lock based solution is developed - which allows to explore the impact of the neater non-blocking solution....
static uint32_t *_LRJOURNAL;

static uint32_t jread,jwrite;
pthread_spinlock_t spinlock_lrj;

#define LOCKLRJ pthread_spin_lock(&spinlock_lrj);
#define UNLOCKLRJ pthread_spin_unlock(&spinlock_lrj);

void locribj_init() {
  pthread_spin_init(&spinlock_lrj,PTHREAD_PROCESS_PRIVATE);
  _LRJOURNAL=malloc(BIG*sizeof(uint32_t));
  jread = 0;
  jwrite = 0;
};

void locribj_push(uint32_t addrref){
  // add a journal entry for this address
  // printf("push %d\n",addrref);
  LOCKLRJ;
  assert(JOURNAL_EMPTY != addrref);
  // printf("jpush - [%d] = %x\n",jwrite,addrref);
  _LRJOURNAL[jwrite++] = addrref;
  jwrite = jwrite % BIG;
  assert(jwrite != jread);  // that would mean the buffer overran
  UNLOCKLRJ;
};

uint32_t locribj_pull() {
  // get a journal entry
  uint32_t rval;
  LOCKLRJ;
  if (jwrite != jread) {
    // printf("jpull - [%d] = %x\n",jread,_LRJOURNAL[jread]);
    rval = _LRJOURNAL[jread++];
    jread = jread % BIG;
  } else
    rval = JOURNAL_EMPTY;
  // printf("pull %d\n",rval);
  UNLOCKLRJ;
  return rval;
};
