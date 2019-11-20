gcc -pthread -std=gnu90 -g -O3 -o bgpparse bgpparse.c timespec.c bigtable.c alloc.c bgpupdate.c locrib.c aspath.c locribjournal.c  phase1.c  phase3.c peer.c peergroup.c ibgpserialize.c copyserialize.c  ebgpserialize.c
# gcc -g -O3 -o bigtabletest bigtabletest.c timespec.c
