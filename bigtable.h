#ifndef __BIGTABLE_H
#define __BIGTABLE_H

#define BIG 1000000
#define TOO_BIG (BIG + 1)
#define RIBSIZE (2 << 25)

// static uint32_t *RIB;
// static uint32_t *bigtable;
extern uint32_t bigtable_index;

void print_prefix(uint8_t l, uint32_t address) ;
void print_prefix64(uint64_t la) ;
// uint32_t encode(uint8_t l, uint32_t a) ;
// uint64_t decode(uint32_t ref) ;
// uint32_t encode64(uint64_t la) ;
void reinit_bigtable() ;
void init_bigtable() ;
void dump_bigtable() ;
uint64_t lookup_bigtable(uint32_t index) ;
uint32_t lookup_RIB(uint8_t l, uint32_t address) ;
uint32_t lookup_RIB64(uint64_t la) ;
#endif
