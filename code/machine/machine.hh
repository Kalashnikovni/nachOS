/// Data structures for simulating the execution of user programs running on
/// top of Nachos.
///
/// User programs are loaded into `mainMemory`; to Nachos, this looks just
/// like an array of bytes.  Of course, the Nachos kernel is in memory too --
/// but as in most machines these days, the kernel is loaded into a separate
/// memory region from user programs, and accesses to kernel memory are not
/// translated or paged.
///
/// In Nachos, user programs are executed one instruction at a time, by the
/// simulator.  Each memory reference is translated, checked for errors, etc.
///
/// DO NOT CHANGE -- part of the machine emulation
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_MACHINE_MACHINE__HH
#define NACHOS_MACHINE_MACHINE__HH


#include "disk.hh"
#include "translation_entry.hh"
#include "threads/utility.hh"


/// Definitions related to the size, and format of user memory.

const unsigned PAGE_SIZE = SECTOR_SIZE;  ///< Set the page size equal to the
                                         ///< disk sector size, for
                                         ///< simplicity.
const unsigned NUM_PHYS_PAGES = 32;
const unsigned MEMORY_SIZE = NUM_PHYS_PAGES * PAGE_SIZE;
const unsigned TLB_SIZE = 4;  ///< if there is a TLB, make it small.

enum ExceptionType {
    NO_EXCEPTION,             // Everything ok!
    SYSCALL_EXCEPTION,        // A program executed a system call.
    PAGE_FAULT_EXCEPTION,     // No valid translation found
    READ_ONLY_EXCEPTION,      // Write attempted to page marked “read-only”
    BUS_ERROR_EXCEPTION,      // Translation resulted in an invalid physical
                              // address.
    ADDRESS_ERROR_EXCEPTION,  // Unaligned reference or one that was beyond
                              // the end of the address space.
    OVERFLOW_EXCEPTION,       // Integer overflow in `add` or `sub`.
    ILLEGAL_INSTR_EXCEPTION,  // Unimplemented or reserved instruction.
    NUM_EXCEPTION_TYPES
};

// User program CPU state.  The full set of MIPS registers, plus a few
// more because we need to be able to start/stop a user program between
// any two instructions (thus we need to keep track of things like load
// delay slots, etc.)

#define STACK_REG       29  ///< User's stack pointer.
#define RET_ADDR_REG    31  ///< Holds return address for procedure calls.
#define HI_REG          32  ///< Double register to hold multiply result.
#define LO_REG          33
#define PC_REG          34  ///< Current program counter.
#define NEXT_PC_REG     35  ///< Next program counter (for branch delay).
#define PREV_PC_REG     36  ///< Previous program counter (for debugging).
#define LOAD_REG        37  ///< The register target of a delayed load.
#define LOAD_VALUE_REG  38  ///< The value to be loaded by a delayed load.
#define BAD_VADDR_REG   39  ///< The failing virtual address on an exception.

#define NUM_GP_REGS     32  ///< 32 general purpose registers on MIPS.
#define NUM_TOTAL_REGS  40

class Instruction;

/// The following class defines the simulated host workstation hardware, as
/// seen by user programs -- the CPU registers, main memory, etc.
///
/// User programs should not be able to tell that they are running on our
/// simulator or on the real hardware, except:
/// * we do not support floating point instructions;
/// * the system call interface to Nachos is not the same as UNIX (10 system
///   calls in Nachos vs. 200 in UNIX!).
/// If we were to implement more of the UNIX system calls, we ought to be
/// able to run Nachos on top of Nachos!
///
/// The procedures in this class are defined in `machine.cc`, `mipssim.cc`,
/// and `translate.cc`.
class Machine {
public:

    /// Initialize the simulation of the hardware for running user programs.
    Machine(bool debug);

    /// De-allocate the data structures.
    ~Machine();

    /// Routines callable by the Nachos kernel.

    /// Run a user program.
    void Run();

    const int *GetRegisters() const;

    /// Read the contents of a CPU register.
    int ReadRegister(unsigned num) const;

    /// store a value into a CPU register.
    void WriteRegister(unsigned num, int value);

    /// Routines internal to the machine simulation -- DO NOT call these.

    /// Run one instruction of a user program.
    void OneInstruction(Instruction *instr);
    /// Do a pending delayed load (modifying a reg).
    void DelayedLoad(unsigned nextReg, int nextVal);

    /// Read or write 1, 2, or 4 bytes of virtual memory (at `addr`).  Return
    /// false if a correct translation could not be found.

    bool ReadMem(unsigned addr, unsigned size, int *value);

    bool WriteMem(unsigned addr, unsigned size, int value);

    /// Translate an address, and check for alignment.
    ///
    /// Set the use and dirty bits in the translation entry appropriately,
    /// and return an exception code if the translation could not be
    /// completed.
    ExceptionType Translate(unsigned virtAddr, unsigned *physAddr,
                            unsigned size, bool writing);

    /// Trap to the Nachos kernel, because of a system call or other
    /// exception.
    void RaiseException(ExceptionType which, unsigned badVAddr);

    /// print the user CPU and memory state.
    void DumpState();

    /// Data structures -- all of these are accessible to Nachos kernel code.
    /// “public” for convenience.
    ///
    /// Note that *all* communication between the user program and the kernel
    /// are in terms of these data structures.

    char *mainMemory;  ///< Physical memory to store user program,
                       ///< code and data, while executing.
    int registers[NUM_TOTAL_REGS];  ///< CPU registers, for executing user
                                    ///< programs.


    /// NOTE: the hardware translation of virtual addresses in the user
    /// program to physical addresses (relative to the beginning of
    /// `mainMemory`) can be controlled by one of:
    /// * a traditional linear page table;
    /// * a software-loaded translation lookaside buffer (tlb) -- a cache of
    ///   mappings of virtual page #'s to physical page #'s.
    ///
    /// If `tlb` is NULL, the linear page table is used.
    /// If `tlb` is non-NULL, the Nachos kernel is responsible for managing
    /// the contents of the TLB.  But the kernel can use any data structure
    /// it wants (eg, segmented paging) for handling TLB cache misses.
    ///
    /// For simplicity, both the page table pointer and the TLB pointer are
    /// public.  However, while there can be multiple page tables (one per
    /// address space, stored in memory), there is only one TLB (implemented
    /// in hardware).  Thus the TLB pointer should be considered as
    /// *read-only*, although the contents of the TLB are free to be modified
    /// by the kernel software.

    TranslationEntry *tlb;  ///< This pointer should be considered
                            ///< “read-only” to Nachos kernel code.

    TranslationEntry *pageTable;
    unsigned pageTableSize;

  private:
    bool singleStep;  ///< Drop back into the debugger after each simulated
                      ///< instruction.
};

extern void ExceptionHandler(ExceptionType which);
  /// Entry point into Nachos for handling user system calls and exceptions.
  /// Defined in `exception.cc`.


/// Routines for converting Words and Short Words to and from the simulated
/// machine's format of little endian.  If the host machine is little endian
/// (DEC and Intel), these end up being NOPs.
///
/// What is stored in each format:
/// * host byte ordering:
///   * kernel data structures;
///   * user registers;
/// * simulated machine byte ordering:
///   * contents of main memory.

unsigned WordToHost(unsigned word);
unsigned short ShortToHost(unsigned short shortword);
unsigned WordToMachine(unsigned word);
unsigned short ShortToMachine(unsigned short shortword);


#endif
