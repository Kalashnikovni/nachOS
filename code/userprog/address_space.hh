/// Data structures to keep track of executing user programs (address
/// spaces).
///
/// For now, we do not keep any information about address spaces.  The user
/// level CPU state is saved and restored in the thread executing the user
/// program (see `thread.hh`).
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_USERPROG_ADDRSPACE__HH
#define NACHOS_USERPROG_ADDRSPACE__HH


#include "threads/system.hh"
#include "vmem/coremap.hh"
#include "filesys/file_system.hh"
#include "machine/translation_entry.hh"
#include "bin/noff.h"
#include <math.h>


const unsigned USER_STACK_SIZE = 1024;  ///< Increase this as necessary!


class AddressSpace {
public:

    /// Create an address space, initializing it with the program stored in
    /// the file `executable`.
    ///
    /// * `executable` is the open file that corresponds to the program.
    AddressSpace(OpenFile *executable);

    /// De-allocate an address space.
    ~AddressSpace();

    /// Initialize user-level CPU registers, before jumping to user code.
    void InitRegisters();

    /// Save/restore address space-specific info on a context switch.

    void SaveState();
    void RestoreState();

    TranslationEntry bringPage(unsigned i);
    void copyPage(unsigned from, unsigned to);

    void LoadSegment(int vaddr);

    void SaveToSwap(int vpn);
    void LoadFromSwap(int vpn, int ppn);

    int getNumPages();

private:

    /// Assume linear page table translation for now!
    TranslationEntry *pageTable;

    /// Number of pages in the virtual address space.
    unsigned numPages;

    /// For demand loading
    OpenFile *executable;
    NoffHeader noffH;

    // SWAP
    OpenFile *swapfile;
};



#endif
