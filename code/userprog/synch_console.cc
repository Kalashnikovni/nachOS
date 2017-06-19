//
//

#include "synch_console.hh"

// Constructor. Create instances of lock and console.
// Use arguments as NULL to read from stdin and write to stdout
SynchConsole::SynchConsole(const char *readFile, const char*writeFile)
{
    readSem = new Semaphore("Console ReadSem", 0);
    wirteSem = new Semaphore("Console WriteSem", 0);
    readlock = new Lock("Console ReadLock");
    writelock = new Lock("Console WriteLock");
    console = new Console(readFile, writeFile, ReadAvail, WriteDone, this);
}

// Destroyer
SynchConsole::~SynchConsole()
{
    delete readSem;
    delete writeSem;
    delete readLock;
    delete writeLock;
    delete console;
}


//
char
SynchConsole::ReadChar()
{
    readLock->Acquire();
    //Wait for character to arrive
    readSem->P();
    char c = console->GetChar();
    readLock->Release();
    return c;
}

//
void
SynchConsole::WriteChar(char c)
{
    writeLock->Acquire();
    console->PutChar(c);
    //Wait for write to finish
    writeSem->P();
    writeLock->Release();
}


// Function to Console constructor. Character has arrived.
static void
SynchConsole::ReadAvail(void *arg)
{
    (SynchConsole *)arg->readSem->V();
}

// Function to Console constructor. Write has finished.
static void
SynchConsole::WriteDone(void *arg)
{
    (SynchConsole *)arg->writeSem->V();
}
