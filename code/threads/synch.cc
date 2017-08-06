/// Routines for synchronizing threads.
///
/// Three kinds of synchronization routines are defined here: semaphores,
/// locks and condition variables (the implementation of the last two are
/// left to the reader).
///
/// Any implementation of a synchronization routine needs some primitive
/// atomic operation.  We assume Nachos is running on a uniprocessor, and
/// thus atomicity can be provided by turning off interrupts.  While
/// interrupts are disabled, no context switch can occur, and thus the
/// current thread is guaranteed to hold the CPU throughout, until interrupts
/// are reenabled.
///
/// Because some of these routines might be called with interrupts already
/// disabled (`Semaphore::V` for one), instead of turning on interrupts at
/// the end of the atomic operation, we always simply re-set the interrupt
/// state back to its original value (whether that be disabled or enabled).
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "threads/synch.hh"
#include "threads/system.hh"


/// Initialize a semaphore, so that it can be used for synchronization.
///
/// * `debugName` is an arbitrary name, useful for debugging.
/// * `initialValue` is the initial value of the semaphore.
Semaphore::Semaphore(const char *debugName, int initialValue)
{
    name  = debugName;
    value = initialValue;
    queue = new List<Thread *>;
}

/// De-allocate semaphore, when no longer needed.
///
/// Assume no one is still waiting on the semaphore!
Semaphore::~Semaphore()
{
    delete queue;
}

/// Wait until semaphore `value > 0`, then decrement.
///
/// Checking the value and decrementing must be done atomically, so we need
/// to disable interrupts before checking the value.
///
/// Note that `Thread::Sleep` assumes that interrupts are disabled when it is
/// called.
void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(INT_OFF);
      // Disable interrupts.

    while (value == 0) {  // Semaphore not available.
        queue->Append(currentThread);  // So go to sleep.
        currentThread->Sleep();
    }
    value--;  // Semaphore available, consume its value.

    interrupt->SetLevel(oldLevel);  // Re-enable interrupts.
}

/// Increment semaphore value, waking up a waiter if necessary.
///
/// As with `P`, this operation must be atomic, so we need to disable
/// interrupts.  `Scheduler::ReadyToRun` assumes that threads are disabled
/// when it is called.
void
Semaphore::V()
{
    Thread   *thread;
    IntStatus oldLevel = interrupt->SetLevel(INT_OFF);

    thread = queue->Remove();
    if (thread != NULL)  // Make thread ready, consuming the `V` immediately.
        scheduler->ReadyToRun(thread);
    value++;
    interrupt->SetLevel(oldLevel);
}

/// Dummy functions -- so we can compile our later assignments.
///
/// Note -- without a correct implementation of `Condition::Wait`, the test
/// case in the network assignment will not work!

/*******************************
 * LOCK
 *******************************/

Lock::Lock(const char *debugName)
{
    name = debugName;
    locksem = new Semaphore(debugName, 1);
    holder = NULL;
}

Lock::~Lock()
{
    delete locksem;
}

void
Lock::Acquire()
{
    if(this->IsHeldByCurrentThread())
        return;

    if (holder != NULL) {
        int currentPriority = currentThread->getPriority();
        if (holder->getPriority() < currentPriority)
            holder->setPriority(currentPriority);
    }
 
    locksem->P();
    holder = currentThread;
}

void
Lock::Release()
{
    if(IsHeldByCurrentThread()) {
        currentThread->setPriority(currentThread->getOriginalPriority());
        locksem->V();
    }
}

bool
Lock::IsHeldByCurrentThread()
{
    return currentThread == holder;
}

/*******************************
 * CONDITION VARIABLE
 *******************************/

Condition::Condition(const char *debugName, Lock *conditionLock)
{
    name = debugName;
    lock = conditionLock;
    list = new List<Semaphore*>();
}

Condition::~Condition()
{
    delete list;
}

void
Condition::Wait()
{
    ASSERT(lock->IsHeldByCurrentThread());
    Semaphore *sem = new Semaphore("ConditionSem", 0);    
    list->Append(sem);

    lock->Release();
    sem->P();
    lock->Acquire();
}

void
Condition::Signal()
{
    if(!(list->IsEmpty())){
        (list->Remove())->V();
    }
}

void
Condition::Broadcast()
{
    while(!(list->IsEmpty())){
        (list->Remove())->V();
    }    
}

/*******************************
 * PORT
 *******************************/

Port::Port(const char* debugName)
{
    name = debugName;
    
    lock = new Lock(debugName);
    condsend = new Condition("Port condition send", lock);
    condrecv = new Condition("Port condition recv", lock);
    inbox = false;
}

Port::~Port()
{
    delete condsend;
    delete condrecv;
    delete lock;
}

void
Port::Send(int message)
{
    lock->Acquire();

    while(inbox){
        condsend->Wait();
    }
    buf = message;
    
    inbox = true;
    condrecv->Signal(); //TODO: Si tiene q ser broadcast xq todos los recv pueden copiar hay que cambiar todo.
    lock->Release();
}

void
Port::Receive(int *message)
{
    lock->Acquire();

    while(!inbox){
        condrecv->Wait();
    }
    *message = buf;

    inbox = false;
    condsend->Signal();
    lock->Release();
}
