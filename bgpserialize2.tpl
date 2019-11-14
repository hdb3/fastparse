void __BODY__ (struct route *route, uint8_t ** q_base , uint16_t q_max ) {

  uint8_t * r_base = (uint8_t*)route + sizeof(struct route);
  uint8_t * r_limit = r_base + route->update_length;

  void * q_limit = *q_base + q_max;

  uint8_t * q = (uint8_t*) *q_base;
  uint8_t * r = (uint8_t*) r_base;

  uint8_t flags, type_code;
  uint16_t attr_length;
  void *attr_ptr;

  void set (uint16_t attr_length, uint8_t flags, uint8_t type_code, void *attr_ptr) {
    if (attr_length<256) {
      *q++ = flags & ~0x10;
      *q++ = type_code;
      *q++ = attr_length & 0xff;
    } else {
      *q++ = flags | 0x10;
      *q++ = type_code;
      *q++ = attr_length >> 8;
      *q++ = attr_length & 0xff;
    };  
    memcpy(q,attr_ptr,attr_length);
    q += attr_length;
  };

  void set4 (uint8_t flags, uint8_t type_code, uint32_t v) {
    *q++ = flags & ~0x10;
    *q++ = type_code;
    *q++ = 4;
    *q = __bswap_32(v);
    q += 4;
  };

  void copy () {
    if (attr_ptr) {
      if (attr_length<256) {
        *q++ = flags & ~0x10;
        *q++ = type_code;
        *q++ = attr_length & 0xff;
      } else {
        *q++ = flags | 0x10;
        *q++ = type_code;
        *q++ = attr_length >> 8;
        *q++ = attr_length & 0xff;
      };  
      memcpy(q,attr_ptr,attr_length);
      q += attr_length;
    };
  };

  void get (uint8_t wanted_type_code) {
    while (r<r_limit-2) { // limit -2 because we need at least a following flags and length field too
      if (r<r_limit-2){
        flags = *r++;
        type_code = *r++;
        attr_length = *r++;
          if (0x10 & flags)
            attr_length = attr_length << 8 | (*r++);
        attr_ptr = r;
        r += attr_length;
        if ( type_code < wanted_type_code)
	  continue;
        else if ( type_code > wanted_type_code) {
          attr_ptr = NULL; // the attribute was not found in the source route
	  break;
	} else { // the attribute was found in the source route
	  break;
	};

      } else attr_ptr = NULL;  // the attribute was not found in the source route (and we reached the end of the route!)
    };  // on exit the attr_length and attr_ptr are correctly set for this attribute.
  };
