//
//TODO: Agregar al makefile


class Coremap : public Bitmap
{
public:
    int Find(AddrSpace, int);
    
private:
    AddrSpace *owner[NUM_PHYS_PAGES];
    int VPN[NUM_PHYS_PAGES];
    int nextVictim = -1;
}

int
Coremap::Find(AddrSpace *o, int vpn)
{
    int free = Bitmap::Find();
    if(free == -1){ //Push victim to swap and take its place
        int victim = SelectVictim();
        ASSERT((0 <= victim) && (victim < NUM_PHYS_PAGES));
        owner[victim] -> SaveToSwap(VPN[victim]);
        free = victim;
    }
    
}

int
SelectVictim()
{
    nextVictim = (nextVictim + 1) % NUM_PHYS_PAGES;
    return nextVictim;
}
