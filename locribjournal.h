#ifndef __LOCRIBJ_H
#define __LOCRIBJ_H

#define JOURNAL_EMPTY 0xffffffff

// static uint32_t *_LRJOURNAL;

// static uint32_t jread,jwrite;

extern inline void locribj_init();
extern inline void locribj_push(uint32_t addrref);
extern inline uint32_t locribj_pull();
#endif
