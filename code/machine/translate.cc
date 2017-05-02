/// Routines to translate virtual addresses to physical addresses.
///
/// Software sets up a table of legal translations.  We look up in the table
/// on every memory reference to find the true physical memory location.
///
/// Two types of translation are supported here.
///
/// Linear page table -- the virtual page # is used as an index into the
/// table, to find the physical page #.
///
/// Translation lookaside buffer -- associative lookup in the table to find
/// an entry with the same virtual page #.  If found, this entry is used for
/// the translation.  If not, it traps to software with an exception.
///
/// In practice, the TLB is much smaller than the amount of physical memory
/// (16 entries is common on a machine that has 1000's of pages).  Thus,
/// there must also be a backup translation scheme (such as page tables), but
/// the hardware does not need to know anything at all about that.
///
/// Note that the contents of the TLB are specific to an address space.
/// If the address space changes, so does the contents of the TLB!
///
/// DO NOT CHANGE -- part of the machine emulation
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "machine.hh"
#include "threads/system.hh"
#include "userprog/address_space.hh"


/// Routines for converting Words and Short Words to and from the simulated
/// machine's format of little endian.  These end up being NOPs when the host
/// machine is also little endian (DEC and Intel).

unsigned
WordToHost(unsigned word)
{
#ifdef HOST_IS_BIG_ENDIAN
     unsigned long result;
     result  = word >> 24 & 0x000000FF;
     result |= word >>  8 & 0x0000FF00;
     result |= word <<  8 & 0x00FF0000;
     result |= word << 24 & 0xFF000000;
     return result;
#else
     return word;
#endif
}

unsigned short
ShortToHost(unsigned short shortword)
{
#ifdef HOST_IS_BIG_ENDIAN
     unsigned short result;
     result  = shortword << 8 & 0xFF00;
     result |= shortword >> 8 & 0x00FF;
     return result;
#else
     return shortword;
#endif
}

unsigned
WordToMachine(unsigned word)
{
    return WordToHost(word);
}

unsigned short
ShortToMachine(unsigned short shortword)
{
    return ShortToHost(shortword);
}


/// Read `size` (1, 2, or 4) bytes of virtual memory at `addr` into
/// the location pointed to by `value`.
///
/// Returns false if the translation step from virtual to physical memory
/// failed.
///
/// * `addr` is the virtual address to read from.
/// * `size` is the number of bytes to read (1, 2, or 4).
/// * `value` is the place to write the result.
bool
Machine::ReadMem(unsigned addr, unsigned size, int *value)
{
    int           data;
    ExceptionType exception;
    unsigned      physicalAddress;

    DEBUG('a', "Reading VA 0x%X, size %u\n", addr, size);

    exception = Translate(addr, &physicalAddress, size, false);
    if (exception != NO_EXCEPTION) {
        machine->RaiseException(exception, addr);
        return false;
    }
    switch (size) {
        case 1:
            data = machine->mainMemory[physicalAddress];
            *value = data;
            break;

        case 2:
            data = *(unsigned short *) &machine->mainMemory[physicalAddress];
            *value = ShortToHost(data);
            break;

        case 4:
            data = *(unsigned *) &machine->mainMemory[physicalAddress];
            *value = WordToHost(data);
            break;

        default: ASSERT(false);
    }

    DEBUG('a', "\tvalue read = %8.8x\n", *value);
    return true;
}

/// Write `size` (1, 2, or 4) bytes of the contents of `value` into virtual
/// memory at location `addr`.
///
/// Returns false if the translation step from virtual to physical memory
/// failed.
///
/// * `addr` is the virtual address to write to.
/// * `size` is the number of bytes to be written (1, 2, or 4).
/// * `value` is the data to be written.
bool
Machine::WriteMem(unsigned addr, unsigned size, int value)
{
    ExceptionType exception;
    unsigned      physicalAddress;

    DEBUG('a', "Writing VA 0x%X, size %u, value 0x%X\n", addr, size, value);

    exception = Translate(addr, &physicalAddress, size, true);
    if (exception != NO_EXCEPTION) {
        machine->RaiseException(exception, addr);
        return false;
    }
    switch (size) {
        case 1:
            machine->mainMemory[physicalAddress]
              = (unsigned char) (value & 0xFF);
            break;

        case 2:
            *(unsigned short *) &machine->mainMemory[physicalAddress]
              = ShortToMachine((unsigned short) (value & 0xFFFF));
            break;

        case 4:
            *(unsigned *) &machine->mainMemory[physicalAddress]
              = WordToMachine((unsigned) value);
            break;

        default:
            ASSERT(false);
    }

    return true;
}

/// Translate a virtual address into a physical address, using
/// either a page table or a TLB.
///
/// Check for alignment and all sorts of other errors, and if everything is
/// ok, set the use/dirty bits in the translation table entry, and store the
/// translated physical address in "physAddr".  If there was an error,
/// returns the type of the exception.
///
/// * `virtAddr" is the virtual address to translate.
/// * `physAddr" is the place to store the physical address.
/// * `size" is the amount of memory being read or written.
/// * `writing` -- if true, check the “read-only” bit in the TLB.
ExceptionType
Machine::Translate(unsigned virtAddr, unsigned *physAddr,
                   unsigned size, bool writing)
{
    unsigned          i, vpn, offset, pageFrame;
    TranslationEntry *entry;

    DEBUG('a', "\tTranslate 0x%X, %s: ",
          virtAddr, writing ? "write" : "read");

    // Check for alignment errors.
    if ((size == 4 && virtAddr & 0x3) || (size == 2 && virtAddr & 0x1)) {
        DEBUG('a', "alignment problem at %u, size %u!\n", virtAddr, size);
        return ADDRESS_ERROR_EXCEPTION;
    }

    // We must have either a TLB or a page table, but not both!
    ASSERT(tlb == NULL || pageTable == NULL);
    ASSERT(tlb != NULL || pageTable != NULL);

    // Calculate the virtual page number, and offset within the page,
    // from the virtual address.
    vpn    = (unsigned) virtAddr / PAGE_SIZE;
    offset = (unsigned) virtAddr % PAGE_SIZE;

    if (tlb == NULL) {        // => page table => `vpn` is index into table.
        if (vpn >= pageTableSize) {
            DEBUG('a',
                  "virtual page # %u too large for page table size %u!\n",
                  virtAddr, pageTableSize);
            return ADDRESS_ERROR_EXCEPTION;
        } else if (!pageTable[vpn].valid) {
            DEBUG('a',
                  "virtual page # %u too large for page table size %u!\n",
                  virtAddr, pageTableSize);
            return PAGE_FAULT_EXCEPTION;
        }
        entry = &pageTable[vpn];
    } else {
        for (entry = NULL, i = 0; i < TLB_SIZE; i++)
            if (tlb[i].valid && tlb[i].virtualPage == vpn) {
                entry = &tlb[i];  // FOUND!
                break;
            }
        if (entry == NULL) {  // Not found.
            DEBUG('a',
                  "*** no valid TLB entry found for this virtual page!\n");
            return PAGE_FAULT_EXCEPTION;  // Really, this is a TLB fault, the
                                          // page may be in memory, but not
                                          // in the TLB.
        }
    }

    if (entry->readOnly && writing) {  // Trying to write to a read-only
                                       // page.
        DEBUG('a', "%u mapped read-only at %u in TLB!\n", virtAddr, i);
        return READ_ONLY_EXCEPTION;
    }
    pageFrame = entry->physicalPage;

    // If the `pageFrame` is too big, there is something really wrong!  An
    // invalid translation was loaded into the page table or TLB.
    if (pageFrame >= NUM_PHYS_PAGES) {
        DEBUG('a', "*** frame %u > %u!\n", pageFrame, NUM_PHYS_PAGES);
        return BUS_ERROR_EXCEPTION;
    }
    entry->use = true;  // Set the `use`, `dirty` bits.
    if (writing)
        entry->dirty = true;
    *physAddr = pageFrame * PAGE_SIZE + offset;
    ASSERT(*physAddr >= 0 && *physAddr + size <= MEMORY_SIZE);
    DEBUG('a', "phys addr = 0x%X\n", *physAddr);
    return NO_EXCEPTION;
}
