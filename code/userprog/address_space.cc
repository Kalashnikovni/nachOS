/// Routines to manage address spaces (executing user programs).
///
/// In order to run a user program, you must:
///
/// 1. Link with the `-N -T 0` option.
/// 2. Run `coff2noff` to convert the object file to Nachos format (Nachos
///    object code format is essentially just a simpler version of the UNIX
///    executable object code format).
/// 3. Load the NOFF file into the Nachos file system (if you have not
///    implemented the file system yet, you do not need to do this last
///    step).
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

<<<<<<< HEAD
=======
#include "userprog/address_space.hh"
#include "threads/system.hh"
>>>>>>> Clock

/// Do little endian to big endian conversion on the bytes in the object file
/// header, in case the file was generated on a little endian machine, and we
/// are re now running on a big endian machine.
static void
SwapHeader(NoffHeader *noffH)
{
    noffH->noffMagic              = WordToHost(noffH->noffMagic);
    noffH->code.size              = WordToHost(noffH->code.size);
    noffH->code.virtualAddr       = WordToHost(noffH->code.virtualAddr);
    noffH->code.inFileAddr        = WordToHost(noffH->code.inFileAddr);
    noffH->initData.size          = WordToHost(noffH->initData.size);
    noffH->initData.virtualAddr   = WordToHost(noffH->initData.virtualAddr);
    noffH->initData.inFileAddr    = WordToHost(noffH->initData.inFileAddr);
    noffH->uninitData.size        = WordToHost(noffH->uninitData.size);
    noffH->uninitData.virtualAddr =
      WordToHost(noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr  = WordToHost(noffH->uninitData.inFileAddr);
}

#ifdef USE_DML
//Load a single page from the noffH where the address addr is
void
AddressSpace::LoadSegment(int vaddr)
{
    DEBUG('z',"Loading segment from addr: %u\n",vaddr);
    Segment segment;
    bool zeroOut = false;
    if((vaddr >= noffH.code.virtualAddr) && (vaddr <= noffH.code.virtualAddr + noffH.code.size)){ //Code segment
        segment = noffH.code;
        DEBUG('z',"Address: [%u] was found to be in code\n", vaddr);
    } else if((vaddr >= noffH.initData.virtualAddr) && (vaddr <= noffH.initData.virtualAddr + noffH.initData.size)){ //InitData segment
        segment = noffH.initData;
        DEBUG('z',"Address: [%u] was found to be in initData\n", vaddr);
    } else { //UninitData or stack segment
        segment = noffH.uninitData;
        zeroOut = true;
        DEBUG('z',"Address: [%u] was found to be in uninitData\n", vaddr);
    }
    
    int vpn = vaddr / PAGE_SIZE;
#ifndef VMEM
    pageTable[vpn].physicalPage = vpages->Find();
#else
    pageTable[vpn].physicalPage = coremap->Find(currentThread->space, vpn);
#endif
    ASSERT(pageTable[vpn].physicalPage >= 0);
    int ppn = pageTable[vpn].physicalPage;
    int pp  = ppn * PAGE_SIZE;

    for (int j = 0; (j < (int)PAGE_SIZE) && (j < executable->Length() - vpn * (int)PAGE_SIZE - segment.inFileAddr); j++){
        char c;

        if(!zeroOut){ // Load the data
            int res = executable->ReadAt(&c, 1, j + segment.inFileAddr + vpn*PAGE_SIZE - segment.virtualAddr);
            ASSERT(res == 1);
        } else { // Zero-Out the uninitData
            c = (char)0;
        }

        int paddr  = pp + j;
        machine->mainMemory[paddr] = c;
    }

    pageTable[vpn].valid = true; //Now that the page is loaded, set it as valid
}
#endif

#ifdef VMEM
// Write a page to SWAP
void
AddressSpace::SaveToSwap(int vpn)
{
    int ppn = pageTable[vpn].physicalPage;
    swapfile->WriteAt(&machine->mainMemory[ppn * PAGE_SIZE], PAGE_SIZE, vpn * PAGE_SIZE);
    /*Invalidar la entrada TLB si es el mismo proceso*/
#ifdef USE_TLB
    if(this == currentThread->space){
        for (int i = 0; i < TLB_SIZE; i++){
            if (machine->tlb[i].valid && machine->tlb[i].virtualPage == vpn) {
                pageTable[machine->tlb[i].virtualPage] = machine->tlb[i];
                machine->tlb[i].valid = false;
            }
        }
    }
#endif
    pageTable[vpn].valid = false;
    pageTable[vpn].physicalPage = -2;
}

// Load a page from SWAP
void
AddressSpace::LoadFromSwap(int vpn, int ppn)
{
    swapfile->ReadAt(&machine->mainMemory[ppn * PAGE_SIZE], PAGE_SIZE, vpn * PAGE_SIZE);
    pageTable[vpn].physicalPage = ppn;
    pageTable[vpn].valid = true;
}
#endif

/// Create an address space to run a user program.
///
/// Load the program from a file `executable`, and set everything up so that
/// we can start executing user instructions.
///
/// Assumes that the object code file is in NOFF format.
///
/// First, set up the translation from program memory to physical memory.
/// For now, this is really simple (1:1), since we are only uniprogramming,
/// and we have a single unsegmented page table.
///
/// * `executable` is the file containing the object code to load into
///   memory.
AddressSpace::AddressSpace(OpenFile *exec)
{
    executable = exec;

    unsigned   size;
    executable->ReadAt((char *) &noffH, sizeof noffH, 0);
    if (noffH.noffMagic != NOFFMAGIC &&
          WordToHost(noffH.noffMagic) == NOFFMAGIC)
        SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

    // How big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size
           + USER_STACK_SIZE;
    // We need to increase the size to leave room for the stack.
    numPages = divRoundUp(size, PAGE_SIZE);
    size = numPages * PAGE_SIZE;

    // Check we are not trying to run anything too big -- at least until we
    // have virtual memory.
    DEBUG('a', "Initializing address space, num pages %u, size %u\n",
          numPages, size);

#ifdef VMEM
    //Create the SWAP file
    int j;
    for(j = 0; j < MAX_NPROCS; j++){
        if(currentThread == ptable[j])
            break;
    }
    ASSERT(ptable[j] == currentThread);
    char sname[128];//5 + (int)(log10(j) + 1) + 1];
    sprintf(sname, "SWAP.%d", j);
    ASSERT(fileSystem->Create(sname, size)); //TODO: DELETE
    swapfile = fileSystem->Open(sname);
#endif

    // First, set up the translation.

    pageTable = new TranslationEntry[numPages]; 
    for (unsigned i = 0; i < numPages; i++) {
        pageTable[i].virtualPage  = i;
#ifdef USE_DML
        pageTable[i].physicalPage = -1;
        pageTable[i].valid        = false;
#else
#ifndef VMEM
        pageTable[i].physicalPage = vpages->Find();
#else
        pageTable[i].physicalPage = coremap->Find(currentThread->space, i);
#endif
        ASSERT(pageTable[i].physicalPage >=0); 
        DEBUG('j',"Assigning physPage: [%d]%d \n",i ,pageTable[i].physicalPage);
        pageTable[i].valid        = true;
#endif
        pageTable[i].use          = false;
        pageTable[i].dirty        = false;
#ifdef VMEM
        coremap->setDirty(pageTable[i].physicalPage);
#endif
        pageTable[i].readOnly     = false;
        // If the code segment was entirely on a separate page, we could
        // set its pages to be read-only.
    }

    DEBUG('a', "Finished initialization...\n");

#ifndef USE_DML
    // Zero out the entire address space, to zero the unitialized data
    // segment and the stack segment.
    for (unsigned i = 0; i < numPages; i++) {
        DEBUG('j', "Zeroing out [%d]%d \n", i, pageTable[i].physicalPage);
        bzero(&(machine->mainMemory[pageTable[i].physicalPage * PAGE_SIZE]), PAGE_SIZE);
    }

    DEBUG('a', "Finished zeroing out the pagetable address... \n");

    // Then, copy in the code and data segments into memory.
    if (noffH.code.size > 0) {
        DEBUG('a', "Initializing code segment...\n");
        for (int j = 0; j < noffH.code.size; j++){
            char c;
            executable->ReadAt(&c, 1, j + noffH.code.inFileAddr);
            int vaddr  = noffH.code.virtualAddr + j; 
            int vpn    = vaddr / PAGE_SIZE;
            int offset = vaddr % PAGE_SIZE;
            int ppn    = pageTable[vpn].physicalPage;
            int pp     = ppn * PAGE_SIZE;
            int paddr  = pp + offset;
            machine->mainMemory[paddr] = c;
        }
    }
    if (noffH.initData.size > 0) {
        DEBUG('a', "Initializing data segment...\n");
        for (int j = 0; j < noffH.initData.size; j++){
            char c;
            executable->ReadAt(&c, 1, j + noffH.initData.inFileAddr);
            int vaddr  = noffH.initData.virtualAddr + j;
            int vpn    = vaddr / PAGE_SIZE;
            int offset = vaddr % PAGE_SIZE; 
            int ppn    = pageTable[vpn].physicalPage;
            int pp     = ppn * PAGE_SIZE;
            int paddr  = pp + offset;
            machine->mainMemory[paddr] = c;
        }
    }
#endif
}

/// Deallocate an address space.
///
/// Nothing for now!
AddressSpace::~AddressSpace()
{
#ifndef VMEM
    unsigned i;
    for(i=0; i < numPages; i++)
        vpages->Clear(pageTable[i].physicalPage);
    delete [] pageTable;
#else
#endif
}

/// Set the initial values for the user-level register set.
///
/// We write these directly into the “machine” registers, so that we can
/// immediately jump to user code.  Note that these will be saved/restored
/// into the `currentThread->userRegisters` when this thread is context
/// switched out.
void
AddressSpace::InitRegisters()
{
    for (unsigned i = 0; i < NUM_TOTAL_REGS; i++)
        machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of `Start`.
    machine->WriteRegister(PC_REG, 0);

    // Need to also tell MIPS where next instruction is, because of branch
    // delay possibility.
    machine->WriteRegister(NEXT_PC_REG, 4);

    // Set the stack register to the end of the address space, where we
    // allocated the stack; but subtract off a bit, to make sure we do not
    // accidentally reference off the end!
    machine->WriteRegister(STACK_REG, numPages * PAGE_SIZE - 16);
    DEBUG('a', "Initializing stack register to %u\n",
          numPages * PAGE_SIZE - 16);
}

/// On a context switch, save any machine state, specific to this address
/// space, that needs saving.
void AddressSpace::SaveState()
{
#ifdef USE_TLB
    DEBUG('b', "Saving state (TLB)\n");
    unsigned i;
    TranslationEntry tlb_entry;
    for(i = 0; i < TLB_SIZE; i++){
	tlb_entry = machine->tlb[i];
	if(tlb_entry.valid == true){
        pageTable[machine->tlb[i].virtualPage] = tlb_entry;
        }
        tlb_entry.valid = false;
    }
#endif
}

/// On a context switch, restore the machine state so that this address space
/// can run.
///
/// For now, tell the machine where to find the page table.
void AddressSpace::RestoreState()
{
#ifdef USE_TLB
DEBUG('b', "Restoring state (TLB)\n");
unsigned i;
for(i = 0; i < TLB_SIZE; i++)
    machine->tlb[i].valid = false;
#else
machine->pageTable     = pageTable;
#endif
machine->pageTableSize = numPages;
}


TranslationEntry AddressSpace::bringPage(unsigned pos)
{
    return pageTable[pos];
}


void AddressSpace::copyPage(unsigned from, unsigned to)
{
    pageTable[to] = machine->tlb[from];
}

int AddressSpace::getNumPages()
{
    return numPages;
}

