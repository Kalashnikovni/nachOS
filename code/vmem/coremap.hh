//COREMAP

#ifndef _COREMAP_HH_
#define _COREMAP_HH_

#include "address_space.hh"
#include "machine.hh"
#include "bitmap.hh"

/*
typedef struct _pageStatus {
    bool used;
    bool dirty;
} pageStatus;
*/

class Coremap: public BitMap
{
public:
    Coremap(int n);
    int Find(AddressSpace* addr, unsigned i);
    
private:
    int SelectVictim();
    AddressSpace *owner[NUM_PHYS_PAGES];
    int ppnToVpn[NUM_PHYS_PAGES];
//    pageStatus ppnstat[NUM_PHYS_PAGES];
    int nextVictim;
    int lastVictim;
    int nitems;
};

#endif
