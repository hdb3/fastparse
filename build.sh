# gcc -g -O2 -o bgpparse bgpparse.c timespec.c
gcc -std=gnu90 -g -O3 -o bgpparse bgpparse.c timespec.c bigtable.c alloc.c bgpupdate.c locrib.c aspath.c locribjournal.c  phase1.c  phase3.c peergroup.c ibgpserialize2.c
gcc -g -O2 -o bigtabletest bigtabletest.c timespec.c
