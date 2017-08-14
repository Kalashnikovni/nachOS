//
//

#include "userprog/synch_console.hh"

// Constructor. Create instances of lock and console.
// Use arguments as NULL to read from stdin and write to stdout
SynchConsole::SynchConsole(const char *readFile, const char*writeFile)
{
    readSem = new Semaphore("Console ReadSem", 0);
    writeSem = new Semaphore("Console WriteSem", 0);
    readLock = new Lock("Console ReadLock");
    writeLock = new Lock("Console WriteLock");
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


// FIXME?: cambie de static void a void, esta bien?
// Function to Console constructor. Character has arrived.
void
SynchConsole::ReadAvail(void *arg)
{
    ((SynchConsole *)arg)->readSem->V();
}

// Function to Console constructor. Write has finished.
void
SynchConsole::WriteDone(void *arg)
{
    ((SynchConsole *)arg)->writeSem->V();
}
