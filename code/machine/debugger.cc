/// Copyright (c) 2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "debugger.hh"
#include "interrupt.hh"
#include "statistics.hh"
#include "threads/system.hh"

#include <ctype.h>


static inline void
PrintPrompt()
{
    const char PROMPT[] = "%u> ";

    printf(PROMPT, stats->totalTicks);
    fflush(stdout);
}

static inline void
PrintHelp()
{
    printf("\
Machine commands:\n\
    <number>     Run until the given timer tick.\n\
    c            Run until completion.\n\
    h, ?         Print this help message.\n\
    s, <return>  Execute one instruction.\n\
    q            Exit.\n\n");
}

static inline const char *
GetGPRegisterName(unsigned i)
{
    static const char *NAMES[] = {
        "ZE", "AT", "V0", "V1", "A0", "A1", "A2", "A3",
        "T0", "T1", "T2", "T3", "T4", "T5", "T6", "T7",
        "S0", "S1", "S2", "S3", "S4", "S5", "S6", "S7",
        "T8", "T9", "K0", "K1", "GP", "SP", "FP", "RA"
    };

    if (i < sizeof NAMES / sizeof *NAMES)
        return NAMES[i];
    else
        return NULL;
}

static inline void
PrintGPRegister(const int *registers, unsigned i)
{
    ASSERT(registers != NULL);

    const char *name = GetGPRegisterName(i);
    if (name)
        printf("%s(%u):\t0x%X", name, i, registers[i]);
    else
        printf("%u:\t0x%X", i, registers[i]);
}

/// Print the user program's CPU state.  We might print the contents of
/// memory, but that seemed like overkill.
static inline void
DumpMachineState(int *previousRegisters)
{
    ASSERT(previousRegisters != NULL);

    const char COLOR_CHANGED_BEGINNING[] = "\033[34m";
    const char COLOR_CHANGED_END[]       = "\033[0m";

    const int *registers = machine->GetRegisters();

    printf("Machine registers:\n");
    for (unsigned i = 0; i < NUM_GP_REGS; i++) {
        if (registers[i] != previousRegisters[i])
            printf(COLOR_CHANGED_BEGINNING);

        printf("\t");
        PrintGPRegister(registers, i);
        if (i % 4 == 3)
            printf("\n");

        if (registers[i] != previousRegisters[i])
            printf(COLOR_CHANGED_END);
    }

    printf("\tHi:\t0x%X",       registers[HI_REG]);
    printf("\tLo:\t0x%X\n",     registers[LO_REG]);
    printf("\tPC:\t0x%X",       registers[PC_REG]);
    printf("\tNextPC:\t0x%X",   registers[NEXT_PC_REG]);
    printf("\tPrevPC:\t0x%X\n", registers[PREV_PC_REG]);
    printf("\tLoad:\t0x%X",     registers[LOAD_REG]);
    printf("\tLoadV:\t0x%X\n",  registers[LOAD_VALUE_REG]);
    printf("\n");

    memcpy(previousRegisters, registers, NUM_TOTAL_REGS * sizeof (int));
}

static inline const char *
GetLine(char *buffer, unsigned size)
{
    if (fgets(buffer, size, stdin) == NULL)
        return NULL;

    // Remove trailing spaces.
    char *p;
    for (p = buffer + strlen(buffer) - 1; isspace(*p); p--);
    *(p + 1) = '\0';

    // Remove leading spaces.
    for (p = buffer; isspace(*p); p++);
    return p;
}

Debugger::Debugger()
{
    runUntilTime = 0;
    memset(previousRegisters, 0, sizeof previousRegisters);
}

/// Primitive debugger for user programs.  Note that we cannot use GDB to
/// debug user programs, since GDB does not run on top of Nachos.  It could,
/// but you would have to implement *a lot* more system calls to get it to
/// work!
///
/// So just allow single-stepping, and printing the contents of memory.
bool
Debugger::Debug()
{
    // Wait until the indicated number of ticks has been reached.
    if (runUntilTime > stats->totalTicks)
        return true;

    interrupt->DumpState();
    DumpMachineState(previousRegisters);

    const char *l;
    char *end;
    unsigned num;
    for (;;) {
        PrintPrompt();

        // Get an input line, and exit if EOF.
        if ((l = GetLine(buffer, BUFFER_SIZE)) == NULL)
            interrupt->Halt();

        if (l[0] == '\0')    // Empty line.
            return true;

        num = strtoul(l, &end, 10);
        if (*end == '\0') {
            runUntilTime = stats->totalTicks + num;
            return true;
        }
        runUntilTime = 0;

        if (l[1] != '\0') {  // Too long line.
            printf("ERROR: command line is too long.\n");
            PrintHelp();
            continue;
        }

        switch (l[0]) {
            case 's':
                return true;
            case 'c':
                return false;
            case 'q':
                interrupt->Halt();
            case 'h':
            case '?':
                PrintHelp();
                break;
            default:
                printf("ERROR: command is invalid.\n");
                PrintHelp();
        }
    }
}
