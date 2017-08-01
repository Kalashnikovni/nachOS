// COREMAP definitions
//

#include "coremap.hh"

Coremap::Coremap(int n) : BitMap(n)
{
    nitems = n;
    nextVictim = -1;
}

int
Coremap::Find(AddressSpace *own, int vpn)
{
    int free = BitMap::Find();
    if(free == -1){ //Push victim to swap and take its place
        int victim = SelectVictim();
        ASSERT((0 <= victim) && (victim < nitems));
        owner[victim] -> SaveToSwap(VPN[victim]);
        free = victim;
    }
    owner[free] = own;
    VPN[free] = vpn;
    return free;
}

// LRU policy
int
Coremap::SelectVictim()
{
    int max;
    for(int i; i < NUM_PHYS_PAGES; i++)

}

// FIFO policy
/*int
Coremap::SelectVictim()
{
    nextVictim = ++nextVictim % nitems;
    return nextVictim;
}*/
