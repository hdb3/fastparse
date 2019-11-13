#define _GNU_SOURCE

#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "getw.h"
#include "libupdates2.h"
#include "nlri2.h"
#include "alloc.h"
#include "timespec.h"
#include "bigtable.h"
#include "locrib.h"
#include "phase1.h"
#include "bgpupdate.h"
