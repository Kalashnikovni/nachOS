/// Bootstrap code to initialize the operating system kernel.
///
/// Allows direct calls into internal operating system functions, to simplify
/// debugging and testing.  In practice, the bootstrap code would just
/// initialize data structures, and start a user program to print the login
/// prompt.
///
/// Most of this file is not needed until later assignments.
///
/// Usage
/// =====
///
///     nachos -d <debugflags> -rs <random seed #>
///            -s -x <nachos file> -c <consoleIn> <consoleOut>
///            -f -cp <unix file> <nachos file>
///            -p <nachos file> -r <nachos file> -l -D -t
///            -n <network reliability> -m <machine id>
///            -o <other machine id>
///            -z
///
/// General options
/// ---------------
///
/// * `-d` -- causes certain debugging messages to be printed (cf.
///   `utility.hh`).
/// * `-rs` -- causes `Yield` to occur at random (but repeatable) spots.
/// * `-z` -- prints version and copyright information, and exits.
///
/// *USER_PROGRAM* options
/// ----------------------
///
/// * `-s` -- causes user programs to be executed in single-step mode.
/// * `-x` -- runs a user program.
/// * `-c` -- tests the console.
///
/// *FILESYS* options
/// -----------------
///
/// * `-f` -- causes the physical disk to be formatted.
/// * `-cp` -- copies a file from UNIX to Nachos.
/// * `-p` -- prints a Nachos file to stdout.
/// * `-r` -- removes a Nachos file from the file system.
/// * `-l` -- lists the contents of the Nachos directory.
/// * `-D` -- prints the contents of the entire file system.
/// * `-t` -- tests the performance of the Nachos file system.
///
/// *NETWORK* options
/// -----------------
///
/// * `-n` -- sets the network reliability.
/// * `-m` -- sets this machine's host id (needed for the network).
/// * `-o` -- runs a simple test of the Nachos network software.
///
/// ----
///
/// NOTE: flags are ignored until the relevant assignment.
///
/// Some of the flags are interpreted here; some in `system.cc`.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "copyright.h"
#include "utility.hh"
#include "system.hh"


// External functions used by this file.

void ThreadTest();
void Copy(const char *unixFile, const char *nachosFile);
void Print(const char *file);
void PerformanceTest(void);
void StartProcess(const char *file);
void ConsoleTest(const char *in, const char *out);
void MailTest(int networkID);

static inline void
PrintVersion()
{
    printf("%s\n%s", VERSION, COPYRIGHT);
}

/// Bootstrap the operating system kernel.
///
/// 1. Check command line arguments.
/// 2. Initialize data structures.
/// 3. (Optionally) call test procedure.
///
/// * `argc` is the number of command line arguments (including the name
///   of the command).  Example:
///       nachos -d +  ->  argc = 3
///
/// * `argv` is an array of strings, one for each command line argument.
///   Example:
///       nachos -d +  ->  argv = {"nachos", "-d", "+"}
int
main(int argc, char **argv)
{
    int argCount;  // The number of arguments for a particular command.

    DEBUG('t', "Entering main");
    Initialize(argc, argv);

    for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount) {
        argCount = 1;
        if (!strcmp(*argv, "-z")) {         // Print version info and exit.
            PrintVersion();
            return 0;
        }
#ifdef USER_PROGRAM
        if (!strcmp(*argv, "-x")) {         // Run a user program.
            ASSERT(argc > 1);
            StartProcess(*(argv + 1));
            argCount = 2;
        } else if (!strcmp(*argv, "-c")) {  // Test the console.
            if (argc == 1)
                ConsoleTest(NULL, NULL);
            else {
                ASSERT(argc > 2);
                ConsoleTest(*(argv + 1), *(argv + 2));
                argCount = 3;
            }
            interrupt->Halt();  // Once we start the console, then Nachos
                                // will loop forever waiting for console
                                // input.
        }
#endif
#ifdef FILESYS
        if (!strcmp(*argv, "-cp")) {        // Copy from UNIX to Nachos.
            ASSERT(argc > 2);
            Copy(*(argv + 1), *(argv + 2));
            argCount = 3;
        } else if (!strcmp(*argv, "-p")) {  // Print a Nachos file.
            ASSERT(argc > 1);
            Print(*(argv + 1));
            argCount = 2;
        } else if (!strcmp(*argv, "-r")) {  // Remove Nachos file.
            ASSERT(argc > 1);
            fileSystem->Remove(*(argv + 1));
            argCount = 2;
        } else if (!strcmp(*argv, "-l"))    // List Nachos directory.
            fileSystem->List();
        else if (!strcmp(*argv, "-D"))      // Print entire filesystem.
            fileSystem->Print();
        else if (!strcmp(*argv, "-t"))      // Performance test.
            PerformanceTest();
#endif
#ifdef NETWORK
        if (!strcmp(*argv, "-o")) {
            ASSERT(argc > 1);
            Delay(2);  // Delay for 2 seconds to give the user time to start
                       // up another nachos.
            MailTest(atoi(*(argv + 1)));
            argCount = 2;
        }
#endif // NETWORK
    }

#ifdef THREADS
    ThreadTest();
#endif

    currentThread->Finish();
      // NOTE: if the procedure `main` returns, then the program `nachos`
      // will exit (as any other normal program would).  But there may be
      // other threads on the ready list.  We switch to those threads by
      // saying that the `main` thread is finished, preventing it from
      // returning.
    return 0;  // Not reached...
}
