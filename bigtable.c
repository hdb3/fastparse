#include "include.h"


/*
 *  explanation:
 *  map every prefix of /24 and smaller in a table of size ~/25
 *  the convention is to encode the length in the upper byte of 4,
 *  remainder is the upper 3 bytes of the address
 *  
 *  the generated /23 address works like this
 *  first, /24 is /24 (top bit is 1) 1xxxxxxxxxx(24)
 *  next, /23 has top bit 0, next is 1 01xxxxxxx(23)
 *  next, /22 has top bits 001xxxxxxxxxxx22)
 * 
 * encoding is simple: shift the address right as a 64 bit value with ..00001 (0000 0001 xxxx xxxx)
 * in the upper word 
 *
 * 0000 25(1) >> n
 *
 * ...00000001(24x) -> 1(24x)
 * ...00000001(23x) -> 01(23x)
*/


void print_prefix(uint8_t l, uint32_t address) {
  printf("%-15s/%d", inet_ntoa((struct in_addr){__bswap_32(address)}),l);
};

void print_prefix64(uint64_t la) {
  print_prefix(la >> 32, la & 0xffffffff);
};

static inline uint32_t encode(uint8_t l, uint32_t a) {
  assert(l < 25);
  return (uint32_t)((uint64_t)(0x100000000LL | (uint64_t)a) >> (32 - l));
};

static inline uint64_t decode(uint32_t ref) {
  uint8_t l = 31 - __builtin_clz(ref);
  uint32_t a = ref << (32 - l);
  return (((uint64_t)l) << 32) | ((uint64_t)a);
};

static inline uint32_t encode64(uint64_t la) {
  return encode(la >> 32, la & 0xffffffff);
};

static uint32_t *RIB;
static uint32_t *bigtable;
uint32_t bigtable_index = 0;

void reinit_bigtable() {
  bigtable_index = 0;
  memset(RIB, 0, 4 * RIBSIZE);
  memset(bigtable, 0, 4 * BIG);
};

pthread_spinlock_t spinlock_bt;
#define LOCKBT pthread_spin_lock(&spinlock_bt);
#define UNLOCKBT pthread_spin_unlock(&spinlock_bt);
void init_bigtable() {
  pthread_spin_init(&spinlock_bt,PTHREAD_PROCESS_PRIVATE);
  bigtable_index = 0;
  RIB = calloc(4, RIBSIZE);
  bigtable = calloc(4, BIG);
};

void dump_bigtable() {

  uint32_t btindex;
  uint64_t la;
  for (btindex = 0; btindex < bigtable_index; btindex++) {
    la = decode(bigtable[btindex]);
    printf("%6d : %s/%d\n", btindex, inet_ntoa((struct in_addr){la & 0xffffffff}), (uint32_t)(la >> 32));
  };
};

uint64_t lookup_bigtable(uint32_t index) {
  if (index < bigtable_index || TOO_BIG == index)
    ;
  else {
    printf("** index=%d limit=%d _msg_count=%ld\n", index, bigtable_index, _msg_count);
    assert(index < bigtable_index || TOO_BIG == index);
  };
  return decode(bigtable[index]);
};

uint32_t lookup_RIB(uint8_t l, uint32_t address) {

  // concurrency concerns:
  //   race on empty slot - 
  //     1) we must protect both the cursor and the slot content after cursor is acquired
  //     this not obviously a simple atomic action....
  //     one simple strategy is a fast (global) lock
  //     an alternate is an atomic test and set first on the slot content, using a fixed canary value,
  //     then acquiring the next cursor value, atomically, and inserting (non atomically).
  //     if a second thread reads the canary value it knows that it arrived in an update cycle by a, possibly lower priority, thread
  //     this is problematic since yielding won't gurantee that the other thread will complete its work immediately and write a valid
  //     index (else why is it already suspended?)
  //     however, this eventuality may be impossible to reach, and the real threat simply the atomic upfdet of the cursor
  //     i will protect agsint this unlikely eventuality with an assertion that the canary is never seen...  
  //     an dsimply make the cursor update atomic


// TODO index is an alias for addrref - change thus to make consisntent usage....
  uint32_t index = encode(l, address);
  assert(index < RIBSIZE);
  uint32_t btindex = RIB[index];
  /* old, simple code
  if (0 == btindex) {
    btindex = bigtable_index++;
    RIB[index] = btindex;
    bigtable[btindex] = index;
  };
  */

  /* new, simple code, uses a lock
  */
  if (0 == btindex) {
    LOCKBT;
    btindex = bigtable_index++;
    RIB[index] = btindex;
    bigtable[btindex] = index;
    UNLOCKBT;
  };
  return btindex;
};

uint32_t lookup_RIB64(uint64_t la) {
  if (25 > (la >> 32))
    return lookup_RIB(la >> 32, la & 0xffffffff);
  else {
    return TOO_BIG;
  };
};
