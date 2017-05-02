/// Miscellaneous useful definitions, including debugging routines.
///
/// The debugging routines allow the user to turn on selected debugging
/// messages, controllable from the command line arguments passed to Nachos
/// (`-d`).  You are encouraged to add your own debugging flags.  The
/// pre-defined debugging flags are:
///
/// * `+` -- turn on all debug messages.
/// * `t` -- thread system.
/// * `s` -- semaphores, locks, and conditions.
/// * `i` -- interrupt emulation.
/// * `m` -- machine emulation (requires *USER_PROGRAM*).
/// * `d` -- disk emulation (requires *FILESYS*).
/// * `f` -- file system (requires *FILESYS*).
/// * `a` -- address spaces (requires *USER_PROGRAM*).
/// * `n` -- network emulation (requires *NETWORK*).
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_THREADS_UTILITY__HH
#define NACHOS_THREADS_UTILITY__HH


/// Miscellaneous useful routines.

//#define min(a, b)  (((a) < (b)) ? (a) : (b))
//#define max(a, b)  (((a) > (b)) ? (a) : (b))

/// Typedef for host memory references, expressed in numerical (integer)
/// form.

#ifdef HOST_x86_64
typedef unsigned long HostMemoryAddress;
#else
typedef unsigned int HostMemoryAddress;
#endif

/// Divide and either round up or down.

inline int divRoundDown(int n, int s)
{
    return n / s;
}

inline int divRoundUp(int n, int s)
{
    return n / s + (n % s > 0 ? 1 : 0);
}

/// This declares the type `VoidFunctionPtr` to be a “pointer to a
/// function taking a pointer argument and returning nothing”.
///
/// With such a function pointer (say it is "func"), we can call it like
/// this:
///    func (arg);
/// or:
///    (*func) (arg);
///
/// This is used by `Thread::Fork` and for interrupt handlers, as well as a
/// couple of other places.
typedef void (*VoidFunctionPtr)(void *arg);

typedef void (*VoidNoArgFunctionPtr)();


// Include interface that isolates us from the host machine system library.
// Requires definition of `bool`, and `VoidFunctionPtr`.
#include "machine/system_dep.hh"

/// Interface to debugging routines.

/// Enable printing debug messages.
extern void DebugInit(const char *flags);

/// Is this debug flag enabled?
extern bool DebugIsEnabled(char flag);

/// Print debug message if `flag` is enabled.
extern void DEBUG(char flag, const char *format, ...);

/// If `condition` is false, print a message and dump core.
///
/// Useful for documenting assumptions in the code.
///
/// NOTE: needs to be a `#define`, to be able to print the location where the
/// error occurred.
#define ASSERT(condition)                                            \
    if (!(condition)) {                                              \
        fprintf(stderr, "Assertion failed: line %d, file \"%s\"\n",  \
                __LINE__, __FILE__);                                 \
        fflush(stderr);                                              \
        Abort();                                                     \
    }


#endif
