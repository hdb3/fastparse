#include "include.h"

int too_long = 0;
uint64_t _msg_count = 0;
uint64_t unique = 0;
uint64_t msg_max;
uint64_t consumed=0;
struct route **adj_rib_in=NULL;

static inline void *alloc_adj_rib_in() {
  return calloc(BIG, sizeof(void*));
};

static inline void zero_adj_rib_in(void*p) {
  memset(p,0,BIG*sizeof(void*));
};

static inline void update_adj_rib_in(uint32_t addrref, struct route *route) {

  uint32_t addrindex = addrref & _LR_INDEX_MASK;  // mask off the overloaded top bits in addrref

/*
  printf("%8ld ",_msg_count);
  print_prefix64(lookup_bigtable(addrindex));
  if (route)
    printf(" route %ld",route->unique);
  else
    printf(" route (nil)");

  printf("\n");
  fflush(stdout);
*/

  struct route * old_route = adj_rib_in[addrindex];
  adj_rib_in[addrindex] = route;
  if (old_route) {
    old_route->use_count--;
    if (0 == old_route->use_count)
      dalloc(old_route);
  };

  if (route) {
    route->use_count++;
    locrib(addrref,route);
  } else if (old_route)
    locrib_withdraw(addrref,route);
  else ; // withdraw for a route we dont have - don't push this!
};

static inline void parse_update(void *p, uint16_t length) {

  struct route *route = NULL;
  struct route * sroute;
  uint64_t msg_count = 0;

  uint16_t withdraw_length = getw16(p);
  uint16_t pathattributes_length = getw16(p + 2 + withdraw_length);
  uint16_t nlri_length = length - withdraw_length - pathattributes_length - 4;
  assert(length >= 4 + withdraw_length + pathattributes_length); // sanity check
  assert(4097>length);
  void *withdrawn = p + 2;
  void *path_attributes = p + 4 + withdraw_length;
  void *nlri = p + 4 + withdraw_length + pathattributes_length;

  if (0==nlri_length && 0==withdraw_length) // must be BGP-MP, or EOR
    return;

  _msg_count++;
  /*
  if (4920368 <= _msg_count){
    fprintf(stderr,"update length: %d  pathattributes length: %d  nlri length %d  withdraw length: %d\n",length, pathattributes_length, nlri_length,withdraw_length);
    fprintf(stderr,"update: ");
    printHex(stderr,p,length);
    fprintf(stderr,"withdrawn: ");
    printHex(stderr,withdrawn,withdraw_length);
    fprintf(stderr,"attributes: ");
    printHex(stderr,path_attributes,pathattributes_length);
    fprintf(stderr,"nlri: ");
    printHex(stderr,nlri,nlri_length);
  };
  */
  if (pathattributes_length && nlri_length) {
    route = alloc(length + sizeof(struct route));
    memset(route, 0, sizeof(struct route));
    // printf("msg count=%-8ld  rte count=%-8ld  length=%4d\r",msg_count,unique,length);
    route->update_length = pathattributes_length;
    memcpy((void*)route + sizeof(struct route), path_attributes, pathattributes_length);
    route->unique = unique++;
    phase1(&route);
    parse_attributes(path_attributes, pathattributes_length, route);
  };

  if (nlri_length) {
    void *nlrip = nlri;
    uint64_t prefix;
    while (nlrip < nlri + nlri_length) {
      prefix = nlri_iter(&nlrip);
      uint32_t addrref = lookup_RIB64(prefix);
      if (TOO_BIG == addrref)
        too_long++;
      else {
        if (nlrip == nlri + nlri_length)  // this is the last prefix in the list
          addrref |= _LR_EOB;
        update_adj_rib_in(addrref,route);
      };
    };
    assert (nlrip == nlri + nlri_length);  // sanity check - will remove when code is tested
  };

  if (withdraw_length) {
    void *withdrawp = withdrawn;
    uint64_t address;
    while (withdrawp < withdrawn + withdraw_length) {
      address = nlri_iter(&withdrawp);
      uint32_t addrref = lookup_RIB64(address);
      if (TOO_BIG == addrref)
        too_long++;
      else {
        if (withdrawp == withdrawn + withdraw_length)  // this is the last prefix in the list
          addrref |= _LR_EOB;
        update_adj_rib_in(addrref,NULL);
      };
    };
    assert (withdrawp == withdrawn + withdraw_length);  // sanity check - will remove when code is tested
  };
};

static unsigned char marker[16] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
int msg_parse(void *base, int64_t length) {

  void *ptr, *limit, *msg;
  uint8_t msg_type;
  uint16_t msg_length;
  // this is a per iteration count - _msg_count is global....
  uint32_t msg_count=0;

  ptr = base;
  limit = base + length;

  reinit_bigtable();
  reinit_alloc();
  locrib_init();
  consumed=0;
  zero_adj_rib_in(adj_rib_in);

  while (ptr < limit && ( msg_max == 0 || msg_count < msg_max)) {
    assert(0 == memcmp(marker, ptr, 16));
    msg_length = getw16(ptr + 16);
    msg_type = getw8(ptr + 18);
    assert(2 == msg_type); // this is an update parser, not a BGP FSM!!!!
    if (msg_length > 23)   // i.e., not an EOR
      parse_update(ptr + 19, msg_length - 19);
    ptr += msg_length;
    msg_count++;
  };

  assert(msg_count == msg_max || ptr == limit);
  consumed =  ptr-base;
  return msg_count;
};

int main(int argc, char **argv) {

  struct stat sb;
  int fd;
  char *fname;
  void *buf;
  int64_t length, message_count;
  int tmp, i, repeat;
  struct timespec tstart, tend;
  double duration;

  fname = argv[1];
  fd = open(fname, O_RDONLY);
  if (-1 == fd) {
    perror("file open error");
    exit(1);
  };
  fstat(fd, &sb);
  length = sb.st_size;
  printf("opened %s file size %ld\n", fname, length);
  buf = mmap(NULL, length, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
  close(fd);

  if (2 < argc && 1 == sscanf(argv[2], "%d", &tmp))
    repeat = tmp;
  else
    repeat = 5;

  if (3 < argc && 1 == sscanf(argv[3], "%d", &tmp))
    msg_max = tmp;
  else
    msg_max = 0;

  init_alloc();
  init_bigtable();
  init_peergroups();
  adj_rib_in = alloc_adj_rib_in();

  tmp = clock_gettime(CLOCK_REALTIME, &tstart);
  message_count = msg_parse(buf, length);
  if (repeat) {
    tmp = clock_gettime(CLOCK_REALTIME, &tstart);
    for (i = 0; i < repeat; i++)
      message_count = msg_parse(buf, length);
  } else
      repeat = 1;

  tmp = clock_gettime(CLOCK_REALTIME, &tend);
  duration = timespec_to_ms(timespec_sub(tend, tstart)) / 1000.0;
  printf("SMALL is %d LARGE is %d\n",SMALL,LARGE);
  printf("read %ld messages\n", message_count);
  printf("complete in %f\n", duration);
  printf("M msgs/sec = %f\n", repeat * message_count / duration / 1000000);
  printf("msgs latency (nsec) = %f\n", duration / repeat / message_count * 1000000000);
  printf("Gbytes/sec = %f\n", repeat * consumed / duration / 1000000000);
  printf("(average message size is %0.2f bytes)\n", (1.0 * consumed) / message_count);
  printf("FIB table size %d\n", bigtable_index);
  report_route_table();
  printf("ignored overlong prefixes: %d\n", too_long / repeat);
};
