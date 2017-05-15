/// Entry point into the Nachos kernel from user programs.
///
/// There are two kinds of things that can cause control to transfer back to
/// here from user code:
///
/// * System calls: the user code explicitly requests to call a procedure in
///   the Nachos kernel.  Right now, the only function we support is `Halt`.
///
/// * Exceptions: the user code does something that the CPU cannot handle.
///   For instance, accessing memory that does not exist, arithmetic errors,
///   etc.
///
/// Interrupts (which can also cause control to transfer from user code into
/// the Nachos kernel) are handled elsewhere.
///
/// For now, this only handles the `Halt` system call.  Everything else core
/// dumps.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "syscall.h"
#include "threads/system.hh"


/// Entry point into the Nachos kernel.  Called when a user program is
/// executing, and either does a syscall, or generates an addressing or
/// arithmetic exception.
///
/// For system calls, the following is the calling convention:
///
/// * system call code in `r2`;
/// * 1st argument in `r4`;
/// * 2nd argument in `r5`;
/// * 3rd argument in `r6`;
/// * 4th argument in `r7`;
/// * the result of the system call, if any, must be put back into `r2`.
///
/// And do not forget to increment the pc before returning. (Or else you will
/// loop making the same system call forever!)
///
/// * `which` is the kind of exception.  The list of possible exceptions is
///   in `machine.hh`.
void
ExceptionHandler(ExceptionType which)
{

    if (which == SYSCALL_EXCEPTION) {
        int type = machine->ReadRegister(2);
        switch(type){
            case SC_Halt:
                DEBUG('a', "Shutdown, initiated by user program.\n");
                interrupt->Halt();
                break;
            case SC_Create:
                int pname = machine->ReadRegister(4);
                char name[128];
                //Create the file
                ReadStringFromUser(pname, name, 128); //Releer la implementacion y ver si funciona cuando corta por exceso de caracteres y si deja un EOF
                filesys->Create(name,0);
                //Increase the PC
                machine->WriteRegister(PREV_PC_REG, machine->ReadRegister(PC_REG);
                machine->WriteRegister(PC_REG, machine->ReadRegister(PC_REG)+4);
                machine->WriteRegister(NEXT_PC_REG, machine->ReadRegister(PC_REG)+4); //FIXME?
                break;
            case SC_Read:
                int pbuf = machine->ReadRegister(4);
                int size = machine->ReadRegister(5);
                OpenFileId id = machine->ReadRegister(6);
                //Obtain file from filesystem
                filesys //Check if fileid exists and what file it corresponds to
                //Read the file
                ReadBufferFromUser(/*TODO*/, pbuf, size); //TODO: If file is not long enough...?!
                //Return how much was read
                machine->WriteRegister(2, /*TODO*/);
                //Increase the PC
                /*TODO*/
                break;
            case SC_Write:
                break;
            case SC_Open:
                int pname = machine->ReadRegister(4);
                char name[128];
                ReadStringFromUser(pname, name, 128);
                //Open the file
                OpenFile *f = filesys->Open(name);
                if(f == NULL) {/*TODO*/}
                OpenFileId fid = /*TODO: Crear la funcion que asocia y crea un file id*/
                //Return the fileid
                machine->WriteRegister(2, fid);
                //Increase the PC
                /*TODO*/
                break;
            case SC_Close:
                break;
            case SC_Exit:
                break;
            case SC_Join:
                break;
            case SC_Exec:
                break;
    } else {
        printf("Unexpected user mode exception %d %d\n", which, type);
        ASSERT(false);
    }
}
