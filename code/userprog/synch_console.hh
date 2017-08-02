//
//

#ifndef SYNCH_CONSOLE_HH
#define SYNCH_CONSOLE_HH

#include "synch.hh"
#include "console.hh"

class SynchConsole {
public:
    // Initialize the raw console and synch mecanisms
    SynchConsole(const char *readFile, const char *writeFile);

    // De-allocate the console and synch mecanisms
    ~SynchConsole();

    // Read/write to a console, returning only once the data is actually
    // read or written.
    char ReadChar();
    void WriteChar(char);

    static void ReadAvail(void*);
    static void WriteDone(void*);

private:
    Console *console;
    
    Semaphore *readSem;
    Semaphore *writeSem;
    Lock *readLock;
    Lock *writeLock;
};

#endif //SYNCH_CONSOLE_HH
