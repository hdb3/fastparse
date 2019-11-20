#include "include.h"

int spare = 0;
int too_long = 0;
uint64_t _msg_count = 0;
uint64_t unique = 0;
uint64_t msg_max;
uint64_t consumed=0;
uint64_t updates=0;
uint64_t update_prefixes=0;
uint64_t received_prefixes=0;

static inline void update_adj_rib_in(struct peer* peer, uint32_t addrref, struct route *route) {

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

  received_prefixes++;
  /*
  // insert for option 1 test
  if (0 && route) {
    if (addrref & _LR_EOB)
      dalloc(route);
  };
  */
  struct route * old_route = peer->adj_rib_in[addrindex];
  peer->adj_rib_in[addrindex] = route;
  // return; // insert for option 1 test
  if (old_route) {
    old_route->use_count--;
    if (0 == old_route->use_count)
      dalloc(old_route);
  };

  if (route) {
    route->use_count++;
    locrib(addrref,route,1); // comment for option (2) test
  } else if (old_route)
    locrib(addrref,route,0);  // comment for option (2) test
  else ; // withdraw for a route we dont have - don't push this!
};

static inline void parse_update(struct peer* peer,void *p, uint16_t length) {

  struct route *route = NULL;
  struct route * sroute;
  uint64_t msg_count = 0;

  uint16_t withdraw_length = getw16(p);
  uint16_t pathattributes_length = getw16(p + 2 + withdraw_length);
  uint16_t nlri_length = length - withdraw_length - pathattributes_length - 4;
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
    phase1(peer, &route);
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
        if (nlrip == nlri + nlri_length) {  // this is the last prefix in the list
          addrref |= _LR_EOB;
	  updates++;
	};
        update_adj_rib_in(peer,addrref,route);
        update_prefixes++;
      };
    };
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
        update_adj_rib_in(peer,addrref,NULL);
      };
    };
  };
};

void init() {
  init_alloc();
  init_bigtable();
  init_peers();
  init_peergroups();
};

void reinit() {
  reinit_peergroups(); // repeat for every cycle so that the files are not concatenated
  reinit_peers();
  reinit_bigtable();
  reinit_alloc();
  locrib_init();
  consumed=0;
  propagated_prefixes = 0;
  received_prefixes = 0;
  updates=0;
  update_prefixes=0;
};

static unsigned char marker[16] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

int buf_parse_peer(struct peer *peer) {

  printf("enter buf_parse_peer\n");
  uint32_t msg_count=0;
  void *ptr, *limit, *msg;
  uint8_t msg_type;
  uint16_t msg_length;

  // base = peer->base;
  // length = peer->length;
  ptr = peer->base;
  limit = ptr + peer->length;

  while (ptr < limit && ( msg_max == 0 || msg_count < msg_max)) {
    assert(0 == memcmp(marker, ptr, 16));
    msg_length = getw16(ptr + 16);
    msg_type = getw8(ptr + 18);
    assert(2 == msg_type); // this is an update parser, not a BGP FSM!!!!
    if (msg_length > 23 && spare <= msg_count)   // i.e., not an EOR
    // if (msg_length > 23)   // i.e., not an EOR
      parse_update(peer,ptr + 19, msg_length - 19);
    ptr += msg_length;
    msg_count++;
  };
  printf("leave buf_parse_peer\n");
  return msg_count;
};

struct buf {void *base; int64_t length;};

void split_buf(int n, struct buf *buf, void *base, int64_t length) {
  buf[0] = (struct buf) {base,length};
};

int buf_parse(void *base, int64_t length) {

  int pn;
  // this is a per iteration count - _msg_count is global....
  uint64_t msg_count=0;

  reinit();

  for (pn=0;pn<npeers;pn++) {
    struct peer *peer = peers+pn;
    peer->base = base;
    peer->length = length;
    // msg_count = buf_parse_peer(peer);
    pthread_create(&(peer->thread_id), NULL, (void*)buf_parse_peer, (void*) peer);
    assert(0 == pthread_join(peer->thread_id,(void**)&msg_count));
  };

  schedule_phase3(1);  // hard force to flush unfinished work

  // TODO fix consumed which will be wrong after doing multipeer
  // consumed =  ptr-base;
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
  struct route **adj_rib_in=NULL;

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
    repeat = 1;

  if (3 < argc && 1 == sscanf(argv[3], "%d", &tmp))
    msg_max = tmp;
  else
    msg_max = 0;

  if (4 < argc && 1 == sscanf(argv[4], "%d", &tmp))
    spare = tmp;
  else
    spare = 0;

  init();

  double min_duration=0, max_duration=0, total_duration=0, ave_duration;
  double latency(double dur) { return dur / message_count * 1000000000; };

  if (repeat>1)
    // discard the first round because it is always anomalous
    // - either higher, except with physical file IO, when usually lower
    message_count = buf_parse(buf, length);
  for (i = 0; i < repeat; i++) {
    printf("%2d/%d: ",i+1,repeat); fflush(stdout);
    tmp = clock_gettime(CLOCK_REALTIME, &tstart);
    message_count = buf_parse(buf, length);
    tmp = clock_gettime(CLOCK_REALTIME, &tend);
    duration = timespec_to_double(timespec_sub(tend, tstart));
    printf("%3.3fs (latency %3.2fnS)\n", duration, latency(duration)); fflush(stdout);
    total_duration += duration;
    max_duration = duration > max_duration ? duration : max_duration;
    if (min_duration)
      min_duration = duration < min_duration ? duration : min_duration;
    else
      min_duration = duration;
  };
  ave_duration = total_duration / repeat;

  printf("complete in %0.2fs (%d cycles)\n", total_duration, repeat);
  printf("read %ld messages  ", message_count);
  printf("(average message size is %0.2f bytes)\n", (1.0 * consumed) / message_count);
  printf("M msgs/sec = %f\n", repeat * message_count / total_duration / 1000000);
  printf("Gbytes/sec = %f\n", repeat * consumed / total_duration / 1000000000);

  printf("\n");
  printf("average: %f  minimum: %f  maximum: %f\n" ,ave_duration , min_duration , max_duration );
  printf("latency(nsec): average: %3.2f  minimum: %3.2f  maximum: %3.2f\n" ,latency(ave_duration) , latency(min_duration) , latency(max_duration) );

  printf("\n");
  printf("FIB table size %d\n", bigtable_index);
  printf("updates %ld  update prefixes %ld ave prefixes/update %1.2f\n", updates, update_prefixes, update_prefixes/(1.0*updates));
  printf("received prefix count: %ld  propagated prefix count %ld, (%2.1f)\n", received_prefixes, propagated_prefixes, (100.0*propagated_prefixes)/(1.0*received_prefixes));
  report_route_table();
  printf("ignored overlong prefixes: %d\n", too_long / repeat);
};
