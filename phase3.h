#ifndef __PHASE3_H
#define __PHASE3_H

// extern inline struct route * read_and_clear(uint32_t addrref);

inline static struct route * read_and_clear(uint32_t addrref) {
 // read and clear the push flag
 // if the route changes whilst we are working then a new action will be scheduled

  struct locrib_entry * lre = &LOCRIB[addrref];
  pthread_spin_lock(&lre->spinlock);
  lre->push_flag = false;
  struct route * rval = lre->head;
  pthread_spin_unlock(&lre->spinlock);
  return rval;
};

extern void init_phase3();
extern inline void schedule_phase3(bool hard);
extern uint64_t propagated_prefixes;
#endif
