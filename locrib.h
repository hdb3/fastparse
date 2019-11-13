#ifndef __LOCRIB_H_
#define __LOCRIB_H_
#define TOP64 0x8000000000000000

#define TOP32 0x80000000
#define SND32 0x40000000

#define _LR_EOB TOP32
#define _LR_NULL_ROUTE SND32
#define _LR_INDEX_MASK 0x3fffffff

extern struct route **LOCRIB;

void locrib_init();


#define ISSET64(route) (TOP64 & (uint64_t) route)
#define ISNOTSET64(route) (!(ISSET64(route)))
#define CLEAR64(route) (~TOP64 & (uint64_t) route)
#define SET64(route) (TOP64 | (uint64_t) route)

void locrib(uint32_t address, struct route *new);
#endif
