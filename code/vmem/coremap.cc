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
        DEBUG('p', "Victim NUMBER: %d\n", victim);
        ASSERT((0 <= victim) && (victim < nitems));
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
    nextVictim = ++nextVictim % nitems;
    return nextVictim;
}
