// COREMAP definitions
//

#include "coremap.hh"

Coremap::Coremap(int n) : BitMap(n)
{
    nitems = n;
    nextVictim = -1;
    victimList = (char*)malloc (sizeof pageStatus * NUM_PHYS_PAGES);
    clearPageStatus(victimList);
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

// Second chance policy
int
Coremap::SelectVictim()
{
    int max;
    for(int i; i < NUM_PHYS_PAGES; i++){
        if (victimList[i].used)
            victimList[i].used = 0;
        else if (victimList[i].dirty)
            /*TODO*/; //saveToSwap (victim)         <<<<<<<<<<<<<<<<<<<<<<<<<<<<<
        else
            return i; 
    }
    if (i == NUM_PHYS_PAGES)
        clearPageStatus(victimList);
}

// FIFO policy
/*int
Coremap::SelectVictim()
{
    nextVictim = ++nextVictim % nitems;
    return nextVictim;
}*/

void setUsed(int page){
    victimList[page].used = 1;
}

void setDirty(int page){
    victimList[page].dirty = 1;
}

void clearPageStatus(pageStatus victimList[]) {
    int i;
    for (i=0; i<NUM_PHYS_PAGES; i++) {
        victimList[i].used = 0;
        victimList[i].dirty = 0;
    
    if (i == NUM_PHYS_PAGES)
        clearPageStatus(victimList);}
}
