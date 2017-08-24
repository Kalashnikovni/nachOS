// COREMAP definitions
//

#include "coremap.hh"

Coremap::Coremap(int n) : BitMap(n)
{
    nitems = n;
    nextVictim = -1;
    lastVictim = 0;
}

int
Coremap::Find(AddressSpace *own, unsigned vpn)
{
    int free = BitMap::Find();
    if(free == -1){ //Push victim to swap and take its place
        int victim = SelectVictim();
        DEBUG('p', "Victim NUMBER: %d\n", victim);
        ASSERT((0 <= victim) && (victim < nitems));
//        owner[victim] -> SaveToSwap(ppnToVpn[victim]);
        free = victim;
        owner[free]->SaveToSwap(ppnToVpn[free]);
    }
    owner[free] = own;
    ppnToVpn[free] = vpn;
    return free;
}
// Politica reloj mejorado.
// Falta poner en used y dirty cuando corresponda
// Seria bueno guardar la pagina en swap cuando 
//  se la carga, para asi no estar escribiendo
//  a cada rato. Se guardaria solo si esta dirty
/*    
int
Coremap::SelectVictim()
{
    int i, index, lastone = (lastVictim + NUM_PHYS_PAGES);
    for(index=lastVictim+1; index < lastone; index++){
        i = index % NUM_PHYS_PAGES; 
        if (ppnstat[i].used)
            ppnstat[i].used = false;
        else if (ppnstat[i].dirty)
            ;
        else{
            lastVictim = i; 
            return i;
        }
    }
    for(index=lastVictim+1; index < lastone; index++){
        i = index % NUM_PHYS_PAGES;
        if(ppnstat[i].dirty)
            ;
        else{
            lastVictim = i;
            return i;
        }
    }
    lastVictim++;
    owner[lastVictim]->SaveToSwap(ppnToVpn[lastVictim]);
    return lastVictim;
}
*/
int
Coremap::SelectVictim()
{
    int i, index, lastone = (lastVictim + NUM_PHYS_PAGES);
    TranslationEntry auxEntry;
    for(index=lastVictim+1; index < lastone; index++){
        i = index % NUM_PHYS_PAGES; 
        auxEntry = owner[i]->bringPage(ppnToVpn[i]);
        if (auxEntry.use)
            auxEntry.use = false;
        else if (auxEntry.dirty)
            ;
        else{
            lastVictim = i; 
            return i;
        }
    }

    for(index=lastVictim+1; index < lastone; index++){
        i = index % NUM_PHYS_PAGES; 
        auxEntry = owner[i]->bringPage(ppnToVpn[i]);
        if (auxEntry.dirty)
            ;
        else{
            lastVictim = i; 
            return i;
        }
    }
    lastVictim = (lastVictim + 1) % NUM_PHYS_PAGES;
//    ASSERT(ppnToVpn[lastVictim] >= 0);
    DEBUG('4', "last: %d ppnToVpn: %d\n", lastVictim, ppnToVpn[lastVictim]);
    return lastVictim;
}


/* FIFO POLICY
int
Coremap::SelectVictim()
{
    nextVictim = ++nextVictim % nitems;
    return nextVictim;
}
*/
