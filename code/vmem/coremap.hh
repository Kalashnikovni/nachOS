//COREMAP

#ifndef _COREMAP_HH_
#define _COREMAP_HH_

#include "address_space.hh"
#include "bitmap.hh"
#include "machine.hh"

typedef struct pair {                                     
    int used;
    int dirty;                                            
} pageStatus;

class Coremap: public BitMap
{
public:
    Coremap(int n);
    int Find(AddressSpace *addr, int i);
    void setUsed(int page);
    void setDirty(int page);    
private:
    int SelectVictim();
    AddressSpace *owner[NUM_PHYS_PAGES];
    int VPN[NUM_PHYS_PAGES];
    int nextVictim;
    int nitems;
    pageStatus victimList[NUM_PHYS_PAGES];           //lista de victimas
    void clearPageStatus(pageStatus*);               //inicializa ""
};



#endif
