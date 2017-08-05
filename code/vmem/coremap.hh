//COREMAP

#ifndef _COREMAP_HH_
#define _COREMAP_HH_

#include "machine/machine.hh"
#include "threads/system.hh"
#include "userprog/address_space.hh"
#include "userprog/bitmap.hh"

typedef struct pair {                                     
    int used;
    int dirty;                                            
} pageStatus;

class AddressSpace; 

class Coremap: public BitMap
{
public:
    Coremap(int n);

    int Find(AddressSpace *addr, int i);
    void SetUsed(int page);
    void SetDirty(int page);    
    int RelatedVPN(int ppn); //toma ppn y devuelve vpn
private:
    int nitems;

    AddressSpace *owner[NUM_PHYS_PAGES];
    int VPN[NUM_PHYS_PAGES];

    int SelectVictim();
    int nextVictim;
    int lastVictim;
    pageStatus victimList[NUM_PHYS_PAGES];           //lista de victimas
    void ClearPageStatus(pageStatus*);               //inicializa ""
};

#endif
