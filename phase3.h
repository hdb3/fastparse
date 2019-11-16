#ifndef __PHASE3_H
#define __PHASE3_H

// extern inline struct route * read_and_clear(uint32_t addrref);
inline static struct route * read_and_clear(uint32_t addrref) {
    // read and clear the push flag
    // if the route changes whilst we are working then a new action will be scheduled
    uint_fast64_t* p = (uint_fast64_t*) &LOCRIB[addrref];
    uint_fast64_t routeptr = atomic_fetch_xor(p,TOP64);
    return (struct route *) routeptr;
};
extern inline void schedule_phase3();
extern uint64_t propagated_prefixes;
#endif
