// COREMAP definitions
//

#include "coremap.hh"

int
Coremap::Find(AddressSpace *own, int vpn)
{
    int free = BitMap::Find();
    if(free == -1){ //Push victim to swap and take its place
        int victim = SelectVictim();
        ASSERT((0 <= victim) && (victim < NUM_PHYS_PAGES));
        owner[victim] -> SaveToSwap(VPN[victim]);
        free = victim;
    }
    owner[free] = own;
    VPN[free] = vpn;
    return free;
}

int
Coremap::SelectVictim()
{
    nextVictim = ++nextVictim % NUM_PHYS_PAGES;
    return nextVictim;
}
