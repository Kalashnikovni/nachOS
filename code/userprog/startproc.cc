//
//
//

#

void
StartProc(*args)
{
    currentThread->space->InitRegisters();
    currentThread->space->RestoreState();

    machine->Run();
}
