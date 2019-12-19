#ifndef __LOCRIB_H_
#define __LOCRIB_H_

#define TOP32 0x80000000

#define _LR_EOB TOP32
#define _LR_INDEX_MASK 0x7fffffff


struct locrib_entry {
  struct route * head;
  pthread_spinlock_t spinlock;
  bool push_flag;
};

extern struct locrib_entry *LOCRIB;

void locrib_init();

extern inline void locrib(uint32_t address, struct route *new, bool update);
#endif
