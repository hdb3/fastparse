#ifndef __LOCRIB_H_
#define __LOCRIB_H_
#define TOP64 0x8000000000000000

#define TOP32 0x80000000

#define _LR_EOB TOP32
#define _LR_INDEX_MASK 0x7fffffff

extern struct route **LOCRIB;

void locrib_init();


#define ISSET64(ROUTE) (TOP64 & (uint64_t) ROUTE)
#define ISNOTSET64(ROUTE) (!(ISSET64(ROUTE)))
#define CLEAR64(ROUTE) (struct route*)(~TOP64 & (uint64_t) ROUTE)
#define SET64(ROUTE) ((struct route*)(TOP64 | (uint64_t) ROUTE))

extern inline void locrib(uint32_t address, struct route *new);
extern inline void locrib_withdraw(uint32_t address, struct route *new);
#endif
