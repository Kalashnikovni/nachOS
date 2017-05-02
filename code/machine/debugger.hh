/// Copyright (c) 2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_MACHINE_DEBUGGER__HH
#define NACHOS_MACHINE_DEBUGGER__HH


#include "machine.hh"


class Debugger {
public:
    Debugger();

    /// Invoke the user program debugger.
    ///
    /// Returns whether to continue single-stepping or not.
    bool Debug();

private:
    static const unsigned BUFFER_SIZE = 80;

    char buffer[BUFFER_SIZE];
    int previousRegisters[NUM_TOTAL_REGS];
    unsigned runUntilTime;  ///< Drop back into the debugger when simulated
                            ///< time reaches this value.
};


#endif
