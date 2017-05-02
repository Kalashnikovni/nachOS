/// Simple test case for the threads assignment.
///
/// Create several threads, and have them context switch back and forth
/// between themselves by calling `Thread::Yield`, to illustrate the inner
/// workings of the thread system.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "system.hh"
#include "synch.hh"

ifdef SEMAPHORE_TEST
    Semaphore sem = new Semaphore ("sem", 3);
endif

/// Loop 10 times, yielding the CPU to another ready thread each iteration.
///
/// * `name` points to a string with a thread name, just for debugging
///   purposes.
void
SimpleThread(void *name)
{
    // Reinterpret arg `name` as a string.
    char *threadName = (char *) name;

    // If the lines dealing with interrupts are commented, the code will
    // behave incorrectly, because printf execution may cause race
    // conditions.
    ifdef SEMAPHORE_TEST
        sem.P();
        DEBUG('s', "SimpleThread %s hizo un P", threadName);
    endif
    for (int num = 0; num < 10; num++) {
        //IntStatus oldLevel = interrupt->SetLevel(IntOff);
        printf("*** thread %s looped %d times\n", threadName, num);
        //interrupt->SetLevel(oldLevel);
        currentThread->Yield();
    }
    ifdef SEMAPHORE_TEST
        sem.V();
        DEBUG('s', "SimpleThread %s hizo un V", threadName);
    endif
    //IntStatus oldLevel = interrupt->SetLevel(IntOff);
    printf(">>> Thread %s has finished\n", threadName);
    //interrupt->SetLevel(oldLevel);
}

/// Set up a ping-pong between several threads.
///
/// Do it by launching ten threads which call `SimpleThread`, and finally
/// calling `SimpleThread` ourselves.
void
ThreadTest()
{
    DEBUG('t', "Entering SimpleTest");

    char *threadname = new char[128];
    strcpy(threadname,"Hilo 1");
    Thread *newThread = new Thread(threadname);
    newThread->Fork(SimpleThread, (void *) threadname);

    SimpleThread((void *) "Hilo 0");
}
