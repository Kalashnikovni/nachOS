///     Implementation of system-dependent interface.  Nachos uses the
///     routines defined here, rather than directly calling the UNIX library,
///     to simplify porting between versions of UNIX, and even to
///     other systems, such as MSDOS.
///
///     On UNIX, almost all of these routines are simple wrappers
///     for the underlying UNIX system calls.
///
///     NOTE: all of these routines refer to operations on the underlying
///     host machine (e.g., the DECstation, SPARC, etc.), supporting the
///     Nachos simulation code.  Nachos implements similar operations,
///     (such as opening a file), but those are implemented in terms
///     of hardware devices, which are simulated by calls to the underlying
///     routines in the host workstation OS.
///
///     This file includes lots of calls to C routines.  C++ requires
///     us to wrap all C definitions with a "extern "C" block".
///     This prevents the internal forms of the names from being
///     changed by the C++ compiler.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "interrupt.hh"
#include "threads/system.hh"

extern "C" {
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/un.h>
#include <sys/mman.h>
#ifdef HOST_i386
#include <sys/time.h>
#endif
#ifdef HOST_LINUX
#include <sys/syscall.h>
#include <unistd.h>
#endif

/// UNIX routines called by procedures in this file

#include <stdlib.h>  // rand(), srand(), etc.
#include <unistd.h>  // unlink(), open(), close(), sleep(), etc.
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>

}


///     Check open file or open socket to see if there are any
///     characters that can be read immediately.  If so, read them
///     in, and return TRUE.
///
///     In the network case, if there are no threads for us to run,
///     and no characters to be read,
///     we need to give the other side a chance to get our host's CPU
///     (otherwise, we will go really slowly, since UNIX time-slices
///     infrequently, and this would be like busy-waiting).  So we
///     delay for a short fixed time, before allowing ourselves to be
///     re-scheduled (sort of like a Yield, but cast in terms of UNIX).
///
///     "fd" -- the file descriptor of the file to be polled
bool
PollFile(int fd)
{
    int            rfd = (1 << fd), wfd = 0, xfd = 0, retVal;
    struct timeval pollTime;

    // Decide how long to wait if there are no characters on the file.
    pollTime.tv_sec = 0;
    if (interrupt->getStatus() == IDLE_MODE)
        pollTime.tv_usec = 20000;  // Delay to let other nachos run.
    else
        pollTime.tv_usec = 0;      // No delay.

    // Poll file or socket.
#ifdef HOST_LINUX
    retVal = select(32, (fd_set *) &rfd, (fd_set *) &wfd, (fd_set *) &xfd,
                    &pollTime);
#else
    retVal = select(32, &rfd, &wfd, &xfd, &pollTime);
#endif

    ASSERT(retVal == 0 || retVal == 1);
    return retVal;  // If 0, no char waiting to be read.
}

/// Open a file for writing.
///
/// Create it if it does not exist; truncate it if it does already exist.
/// Return the file descriptor.
///
/// * `name` is the file name.
int
OpenForWrite(const char *name)
{
    int fd = open(name, O_RDWR | O_CREAT | O_TRUNC, 0666);

    ASSERT(fd >= 0);
    return fd;
}

/// Open a file for reading or writing.
///
/// Return the file descriptor, or error if it does not exist.
///
/// * `name` is the file name.
int
OpenForReadWrite(const char *name, bool crashOnError)
{
    int fd = open(name, O_RDWR, 0);

    ASSERT(!crashOnError || fd >= 0);
    return fd;
}

/// Read characters from an open file.
///
/// Abort if read fails.
void
Read(int fd, char *buffer, int nBytes)
{
    int retVal = read(fd, buffer, nBytes);
    ASSERT(retVal == nBytes);
}

/// Read characters from an open file, returning as many as are available.
int
ReadPartial(int fd, char *buffer, int nBytes)
{
    return read(fd, buffer, nBytes);
}


/// Write characters to an open file.
///
/// Abort if write fails.
void
WriteFile(int fd, const char *buffer, int nBytes)
{
    int retVal = write(fd, buffer, nBytes);
    ASSERT(retVal == nBytes);
}

/// Change the location within an open file.
///
/// Abort on error.
void
Lseek(int fd, int offset, int whence)
{
    int retVal = lseek(fd, offset, whence);
    DEBUG('z',"fd: %d, LSEEK: %s\n",fd, strerror(errno));
    ASSERT(retVal >= 0);
}

/// Report the current location within an open file.
int
Tell(int fd)
{
#if defined(HOST_i386) || defined(HOST_LINUX)
    // 386BSD does not have the `tell` system call.
    return lseek(fd, 0, SEEK_CUR);
#else
    return tell(fd);
#endif
}

/// Close a file.
///
/// Abort on error.
void
Close(int fd)
{
    int retVal = close(fd);
    ASSERT(retVal >= 0);
}

/// Delete a file.
bool
Unlink(const char *name)
{
    return unlink(name);
}

/// Open an interprocess communication (IPC) connection.
///
/// For now, just open a datagram port where other Nachos (simulating
/// workstations on a network) can send messages to this Nachos.
int
OpenSocket()
{
    int sockID;

    sockID = socket(AF_UNIX, SOCK_DGRAM, 0);
    ASSERT(sockID >= 0);

    return sockID;
}

/// Close the IPC connection.
void
CloseSocket(int sockID)
{
    (void) close(sockID);
}

/// Initialize a UNIX socket address -- magical!
static void
InitSocketName(struct sockaddr_un *uname, const char *name)
{
    uname->sun_family = AF_UNIX;
    strcpy(uname->sun_path, name);
}

/// Give a UNIX file name to the IPC port, so other instances of Nachos can
/// locate the port.
void
AssignNameToSocket(const char *socketName, int sockID)
{
    struct sockaddr_un uName;
    int                retVal;

    (void) unlink(socketName);  // In case it is still around from last time.

    InitSocketName(&uName, socketName);
    retVal = bind(sockID, (struct sockaddr *) &uName, sizeof uName);
    ASSERT(retVal >= 0);
    DEBUG('n', "Created socket %s\n", socketName);
}

/// Delete the UNIX file name we assigned to our IPC port, on cleanup.
void
DeAssignNameToSocket(const char *socketName)
{
    (void) unlink(socketName);
}

/// Return true if there are any messages waiting to arrive on the IPC port.
bool
PollSocket(int sockID)
{
    return PollFile(sockID);  // On UNIX, socket ID's are just file ID's.
}

/// Read a fixed size packet off the IPC port.
///
/// Abort on error.
void
ReadFromSocket(int sockID, char *buffer, int packetSize)
{
    int retVal;
    // Comentado para evitar error de compilacion Red Hat 9 (2004)
    //extern int errno;
    struct sockaddr_un uName;
    int size = sizeof uName;

    retVal = recvfrom(sockID, buffer, packetSize, 0,
                      (struct sockaddr *) &uName, (socklen_t *) &size);

    if (retVal != packetSize) {
        perror("in recvfrom");
        // Comentado para evitar error de compilacion Red Hat 9 (2004)
        //printf("called: %X, got back %d, %d\n", buffer, retVal, errno);
    }
    ASSERT(retVal == packetSize);
}

/// Transmit a fixed size packet to another Nachos' IPC port.
///
/// Abort on error.
void
SendToSocket(int sockID, const char *buffer,
             int packetSize, const char *toName)
{
    struct sockaddr_un uName;
    int                retVal;

    InitSocketName(&uName, toName);
#ifdef HOST_LINUX
    retVal = sendto(sockID, buffer, packetSize, 0,
                    (const struct sockaddr *) &uName, sizeof uName);
#else
    retVal = sendto(sockID, buffer, packetSize, 0,
                    (char *) &uName, sizeof uName);
#endif

    ASSERT(retVal == packetSize);
}


/// Arrange that `func` will be called when the user aborts (e.g., by hitting
/// ctl-C).
void
CallOnUserAbort(VoidNoArgFunctionPtr func)
{
    typedef void (*SignalHandler)(int);
    (void) signal(SIGINT, (SignalHandler) func);
}

/// Put the UNIX process running Nachos to sleep for `seconds` seconds, to
/// give the user time to start up another invocation of Nachos in a
/// different UNIX shell.
void
Delay(int seconds)
{
    (void) sleep((unsigned) seconds);
}

/// Quit and drop core.
void
Abort()
{
    abort();
}

/// Quit without dropping core.
void
Exit(int exitCode)
{
    exit(exitCode);
}

/// Initialize the pseudo-random number generator.
///
/// We use the now obsolete `srand` and `rand` because they are more
/// portable!
void
RandomInit(unsigned seed)
{
    srand(seed);
}

/// Return a pseudo-random number.
int
Random()
{
    return rand();
}

/// Return an array, with the two pages just before and after the array
/// unmapped, to catch illegal references off the end of the array.
/// Particularly useful for catching overflow beyond fixed-size thread
/// execution stacks.
///
/// Note: Just return the useful part!
///
/// * `size` -- amount of useful space needed (in bytes).
char *
AllocBoundedArray(int size)
{
    int   pgSize = getpagesize();
    char *ptr    = new char[pgSize * 2 + size];

#ifndef HOST_LINUX
    mprotect(ptr, pgSize, 0);
    mprotect(ptr + pgSize + size, pgSize, 0);
#endif
    return ptr + pgSize;
}

/// Deallocate an array of integers, unprotecting its two boundary pages.
///
/// * `ptr` is the array to be deallocated.
/// * `size` is the amount of useful space in the array (in bytes).
void
DeallocBoundedArray(const char *ptr, int size)
{
    int pgSize = getpagesize();

#ifndef HOST_LINUX
    mprotect(ptr - pgSize, pgSize, PROT_READ | PROT_WRITE | PROT_EXEC);
    mprotect(ptr + size,   pgSize, PROT_READ | PROT_WRITE | PROT_EXEC);
#endif
    delete [] (ptr - pgSize);
}
