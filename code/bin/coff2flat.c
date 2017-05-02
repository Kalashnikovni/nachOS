/// Copyright (c) 1992      The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

/// This program reads in a COFF format file, and outputs a flat file --
/// the flat file can then be copied directly to virtual memory and executed.
/// In other words, the various pieces of the object code are loaded at
/// the appropriate offset in the flat file.
///
/// Assumes coff file compiled with -N -T 0 to make sure it is not shared
/// text.


#include "coff.h"
#include "threads/copyright.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// NOTE -- once you have implemented large files, it is ok to make this
// bigger!
#define STACK_SIZE              1024  // In bytes.
#define ReadStructOrDie(fd, s)  ReadOrDie(fd, (char *) &(s), sizeof (s))

/// Read and check for error.
static void
ReadOrDie(int fd, char *buffer, unsigned numBytes)
{
    if (read(fd, buffer, numBytes) != numBytes) {
        fprintf(stderr, "File is too short\n");
        exit(1);
    }
}

/// Write and check for error.
static void
WriteOrDie(int fd, const char *buffer, unsigned numBytes)
{
    if (write(fd, buffer, numBytes) != numBytes) {
        fprintf(stderr, "Unable to write file\n");
        exit(1);
    }
}

/// Do the real work.
void
main(int argc, char *argv[])
{
    int            fdIn, fdOut, numsections, i, top, tmp;
    struct filehdr fileh;
    struct aouthdr systemh;
    struct scnhdr *sections;
    char          *buffer;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <coffFileName> <flatFileName>\n",
                argv[0]);
        exit(1);
    }

    // Open the object file (input).
    fdIn = open(argv[1], O_RDONLY, 0);
    if (fdIn == -1) {
        perror(argv[1]);
        exit(1);
    }

    // Open the flat file (output).
    fdOut = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fdOut == -1) {
        perror(argv[2]);
        exit(1);
    }

    // Read in the file header and check the magic number.
    ReadStructOrDie(fdIn, fileh);
    if (fileh.f_magic != MIPSELMAGIC) {
        fprintf(stderr, "File is not a MIPSEL COFF file\n");
        exit(1);
    }

    // Read in the system header and check the magic number.
    ReadStructOrDie(fdIn, systemh);
    if (systemh.magic != OMAGIC) {
        fprintf(stderr, "File is not a OMAGIC file\n");
        exit(1);
    }

    // Read in the section headers.
    numsections = fileh.f_nscns;
    sections = malloc(fileh.f_nscns * sizeof (struct scnhdr));
    ReadOrDie(fdIn, (char *) sections,
              fileh.f_nscns * sizeof (struct scnhdr));

    // Copy the segments in.
    printf("Loading %d sections:\n", fileh.f_nscns);
    for (top = 0, i = 0; i < fileh.f_nscns; i++) {
        printf("\t\"%s\", filepos 0x%X, mempos 0x%X, size 0x%X\n",
               sections[i].s_name, sections[i].s_scnptr,
               sections[i].s_paddr, sections[i].s_size);
        if (sections[i].s_paddr + sections[i].s_size > top)
            top = sections[i].s_paddr + sections[i].s_size;
        // No need to copy if `.bss`.
        if (strcmp(sections[i].s_name, ".bss")
              && strcmp(sections[i].s_name, ".sbss")) {
            lseek(fdIn, sections[i].s_scnptr, 0);
            buffer = malloc(sections[i].s_size);
            ReadOrDie(fdIn, buffer, sections[i].s_size);
            WriteOrDie(fdOut, buffer, sections[i].s_size);
            free(buffer);
        }
    }

    // Put a blank word at the end, so we know where the end is!
    printf("Adding stack of size: %d\n", STACK_SIZE);
    lseek(fdOut, top + STACK_SIZE - 4, 0);
    tmp = 0;
    WriteOrDie(fdOut, (char *) &tmp, 4);

    close(fdIn);
    close(fdOut);
}
