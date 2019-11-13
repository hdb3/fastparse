#ifndef __BGPUPDATE_H_
#define __BGPUPDATE_H_

// static inline void parse_attribute(uint8_t type_code, void *p, uint16_t length, struct route *route);
static inline void parse_attributes(void *p, uint16_t length, struct route *r);
#endif
