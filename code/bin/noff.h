/// Data structures defining the Nachos Object Code Format
///
/// Basically, we only know about three types of segments: code (read-only),
/// initialized data, and unitialized data.

#ifndef NACHOS_BIN_NOFF__H
#define NACHOS_BIN_NOFF__H


#define NOFFMAGIC  0xBADFAD  // Magic number denoting Nachos object code
                             // file.

typedef struct segment {
    int virtualAddr;  // Location of segment in virtual address space.
    int inFileAddr;   // Location of segment in this file.
    int size;         // Size of segment.
} Segment;

typedef struct noffHeader {
    int noffMagic;       // Should be `NOFFMAGIC`.
    Segment code;        // Executable code segment.
    Segment initData;    // Initialized data segment.
    Segment uninitData;  // Uninitialized data segment -- should be zeroed
                         // before use.
} NoffHeader;


#endif
