void __BODY__ (struct route *route, uint8_t ** q_base , uint16_t q_max ) {

  uint8_t * r_base = (uint8_t*)route + sizeof(struct route);
  uint8_t * r_limit = r_base + route->update_length;

  uint8_t * q_limit = *q_base + q_max;

  uint8_t * q = (uint8_t*) *q_base;
  uint8_t * r = (uint8_t*) r_base;

  uint8_t flags, type_code;
  uint16_t attr_length;
  uint8_t *attr_ptr;

  void set (uint16_t attr_length, uint8_t flags, uint8_t type_code, uint8_t *attr_ptr) {
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

  void prepend (uint32_t as) {
    if (attr_ptr) {
    if(2 != *attr_ptr)
      printf("seg type = %2x\n",*attr_ptr);
    assert(2 == *attr_ptr);
    uint8_t old_seq_length = *(attr_ptr+1);
    assert(255 >  old_seq_length);
    uint16_t new_attr_length = 4 + attr_length; // aasumes firts seg is a non full seq

      if (new_attr_length<256) {
        *q++ = flags & ~0x10;
        *q++ = type_code;
        *q++ = new_attr_length & 0xff;
      } else {
        *q++ = flags | 0x10;
        *q++ = type_code;
        *q++ = new_attr_length >> 8;
        *q++ = new_attr_length & 0xff;
      };
      *q++ = 2;
      *q++ = old_seq_length+1;
      * (uint32_t*) q = __bswap_32(as);
      q += 4;
      memcpy(q,attr_ptr+6,attr_length-2);
      q += attr_length-2;
    };
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
    // assert(q < q_limit);
  };

  void get (uint8_t wanted_type_code) {

  uint8_t *tmp_attr_ptr;
  attr_ptr = NULL;  // default outcome

    while (r<r_limit-2) { // limit -2 because we need at least a following flags and length field too
      flags = *r++;
      type_code = *r++;
      attr_length = *r++;
        if (0x10 & flags)
          attr_length = attr_length << 8 | (*r++);
      tmp_attr_ptr = r;
      r += attr_length;
      if (type_code < wanted_type_code) // the attribute was not found in the source routebut we have not yet gone past its slot
        continue;
      else if (type_code == wanted_type_code) {  // the attribute was found in the source route
        attr_ptr = tmp_attr_ptr;
        break;
      } else // the attribute was not found in the source route and we have gone past its slot
        break;
    };
  };
