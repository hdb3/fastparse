#include "include.h"

struct route * read_and_clear(uint32_t addrref) {
    // read and clear the push flag
    // if the route changes whilst we are working then a new action will be scheduled
    uint_fast64_t* p = (uint_fast64_t*) &LOCRIB[addrref];
    uint_fast64_t routeptr = atomic_fetch_xor(p,TOP64);
    return (struct route *) routeptr;
};

static uint8_t tx_buffer[8192] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff };
static int over_length_detected = 0;

void schedule_phase3() {
  uint32_t addrref;
  uint32_t addrreftable [ 4096 ];
  uint16_t table_index;
  struct route * route1, *route2=NULL;
  uint8_t *txp;
  
  do {

    if (NULL == route2) {
      addrref = locribj_pull();
      if (JOURNAL_EMPTY == addrref)
        break;
      route1 = read_and_clear(addrref);
      route2 = route1;
    } else
      route1 = route2;

    table_index = 0;
    while (route2 == route1) {
        addrreftable[table_index++] = addrref;
        addrref = locribj_pull();
        if (JOURNAL_EMPTY != addrref)
          route2 = read_and_clear(addrref);
	else
          route2 = NULL;
    };
  // inner loop complete, either becuase the routes in the stream are now different
  // or the stream is ended
 
  // regardless, process this block first

    uint32_t index;
    uint16_t pix;

    tx_buffer[18] = 2; // type_code = UPDATE
    if (NULL==route1){ // this is a route withdraw
      txp = tx_buffer+23;
      for (index=0; index < table_index; index++)
        build_nlri(&txp,lookup_bigtable(addrreftable[index]));
      uint16_t withdraw_length = txp - (tx_buffer+21);
      uint16_t update_length = withdraw_length + 23;
      putw16(tx_buffer+16,update_length);
      putw16(tx_buffer+19,withdraw_length);
      putw16(txp, 0); // path attribute length

      for (pix=0;pix<npeergroups;pix++) {
	if (peergroups[pix].file) // allow testing to build the update but never write it if the destination is not open
          assert (1 == fwrite(tx_buffer,update_length, 1, peergroups[pix].file));
      };

    } else { // this is a route update
      putw16(tx_buffer+19,0); // withdrwa length zero for all updates
      for (pix=0;pix<npeergroups;pix++) {
	struct peergroup *pg = &peergroups[pix];
        txp = tx_buffer+23;
        pg->serialize(CLEAR64(route1),&txp,4064);
        uint16_t attribute_length = txp - (tx_buffer+23);
        putw16(tx_buffer+21, attribute_length);
        for (index=0; index < table_index; index++)
          build_nlri(&txp,lookup_bigtable(addrreftable[index]));
	// TODO
	// build two or more updates when the length limit is exceeded
	// this needs to extend build_nlri to take a limit, etc....
        uint16_t update_length = txp - tx_buffer;
        uint16_t nlri_length = txp - tx_buffer - attribute_length - 23;
	// assert(4097>update_length);
        putw16(tx_buffer+16,update_length);

	if (peergroups[pix].file) // allow testing to build the update but never write it if the destination is not open
          assert (1 == fwrite(tx_buffer,update_length, 1, peergroups[pix].file));
	if(4096<update_length && !(over_length_detected)) {
          over_length_detected=1;
	  printf("over length exception (%d)\n",update_length);
	  /*
          int fd = open("exception.bin",O_WRONLY | O_CREAT, 0666 );
          putw16(tx_buffer+16,4096);
          int tmp = write(fd,tx_buffer,update_length);
	  close(fd);
	  exit(1);
	  assert(4097>update_length);
	  */
	};
      };
    };
  } while (JOURNAL_EMPTY != addrref);
};
