//COREMAP

#ifndef _COREMAP_HH_
#define _COREMAP_HH_

#include "address_space.hh"
#include "machine.hh"
#include "bitmap.hh"

class Coremap: public BitMap
{
public:
    Coremap(int n);
    int Find(AddressSpace* addr, int i);
    
private:
    int SelectVictim();
    AddressSpace *owner[NUM_PHYS_PAGES];
    int VPN[NUM_PHYS_PAGES];
    int nextVictim;
    int nitems;
};

#endif
