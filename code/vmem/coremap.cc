// COREMAP definitions
//

#include "vmem/coremap.hh"

Coremap::Coremap(int n) : BitMap(n)
{
    nitems = n;
    nextVictim = -1;
    lastVictim = 0;
    //victimList = (char*)malloc (2 * sizeof(int) * NUM_PHYS_PAGES);
    ClearPageStatus(victimList);
}

int
Coremap::Find(AddressSpace *own, int vpn)
{
    int free = BitMap::Find();
    if(free == -1){ //Push victim to swap and take its place
        int victim = SelectVictim();
        DEBUG('p', "victim number with value %d\n", victim); 
        ASSERT((0 <= victim) && (victim < NUM_PHYS_PAGES));
        free = victim;
    }
    owner[free] = own;
    VPN[free] = vpn;
    return free;
}

// Second chance policy
/*
int
Coremap::SelectVictim()
{
    int limit = lastVictim + NUM_PHYS_PAGES;     // "NUM_PHYS_PAGES, starting from the last"
    int i, inpp;

    for(i=lastVictim; i < limit; i++){
        inpp = i % NUM_PHYS_PAGES;               // Save keep the remanent so we can indent
        if (victimList[inpp].used)
            victimList[inpp].used = 0;
        else{
            if (!victimList[inpp].dirty){
                lastVictim = inpp; 
                return inpp;
            }
        }
    } 
    for(i=lastVictim; i < limit; i++){
        inpp = i % NUM_PHYS_PAGES;               // Save keep the remanent so we can indent
        if (victimList[inpp].dirty){
            owner[inpp]->SaveToSwap(RelatedVPN(inpp));
            return inpp;
        }    
    } 

    ASSERT(false);
//    if (i == NUM_PHYS_PAGES)
//        selectVictimForcingSwap(victimList);
}*/


// FIFO policy
int
Coremap::SelectVictim()
{
    nextVictim = ++nextVictim % nitems;
    owner[nextVictim] -> SaveToSwap(VPN[nextVictim]);
    return nextVictim;
}

void 
Coremap::SetUsed(int page){
    victimList[page].used = 1;
}

void 
Coremap::SetDirty(int page){
    victimList[page].dirty = 1;
}

int Coremap::RelatedVPN(int ppn){
    return VPN[ppn];
}

void 
Coremap::ClearPageStatus(pageStatus victimList[]) {
    int i;
    for (i=0; i<NUM_PHYS_PAGES; i++) {
        victimList[i].used = 0;
        victimList[i].dirty = 0;
    } 
}
