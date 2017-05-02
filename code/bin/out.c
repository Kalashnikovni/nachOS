/// Looking at a.out formats
///
/// First task:
/// Look at mips COFF stuff:
/// Print out the contents of a file and do the following:
///    For data, print the value and give relocation information
///    For code, disassemble and give relocation information
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "coff.h"
#include "extern/reloc.h"
#include "extern/syms.h"
#include "threads/copyright.h"

#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define ReadStruct(f, s) (fread(&(s), sizeof (s), 1, (f)) == 1)

#define MAX_DATA    10000
#define MAX_RELOCS  1000

struct data {
    uint32_t     data[MAX_DATA];
    struct reloc reloc[MAX_RELOCS];
    int          length;
    int          relocs;
};

#define MAX_SECTIONS  10
#define MAX_SYMBOLS   300
#define MAX_SSPACE    20000

struct filehdr fileHeader;
struct aouthdr aoutHeader;
struct scnhdr sectionHeader[MAX_SECTIONS];
struct data section[MAX_SECTIONS];

static HDRR symbolHeader;
static EXTR symbols[MAX_SYMBOLS];
static char sspace[MAX_SSPACE];

static const char *SYMBOL_TYPE[] = {
    "Nil", "Global", "Static", "Param", "Local", "Label", "Proc", "Block",
    "End", "Member", "Type", "File", "Register", "Forward", "StaticProc",
    "Constant" };

static const char *STORAGE_CLASS[] = {
    "Nil", "Text", "Data", "Bss", "Register", "Abs", "Undefined", "CdbLocal",
    "Bits", "CdbSystem", "RegImage", "Info", "UserStruct", "SData", "SBss",
    "RData", "Var", "Common", "SCommon", "VarRegister", "Variant",
    "SUndefined", "Init" };

static int column = 1;

void
MyPrintf(const char *format, ...)
{
    va_list ap;
    char buffer[100];

    va_start(ap, format);
    vsprintf(buffer, format, ap);
    va_end(ap);

    printf("%s", buffer);

    for (unsigned i = 0; buffer[i] != '\0'; i++) {
        if (buffer[i] == '\n')
            column = 1;
        else if (buffer[i] == '\t')
            column = ((column + 7) & ~7) + 1;
        else
            column += 1;
    }
}

static int
MyTab(int n)
{
    while (column < n) {
        putchar(' ');
        column++;
    }
    return column == n;
}

static const char *SECTION_NAME[] = {
    "(null)", ".text", ".rdata", ".data", ".sdata", ".sbss", ".bss",
    ".init", ".lit8", ".lit4"
};

static const char *RELOC_TYPE[] = {
    "abs", "16", "32", "26", "hi16", "lo16", "gpdata", "gplit"
};

static void
PrintReloc(int vaddr, int i, int j)
{
    for (unsigned k = 0; k < section[i].relocs; k++) {
        struct reloc *rp;
        rp = &section[i].reloc[k];

        if (vaddr == rp->r_vaddr) {
            MyTab(57);
            if (rp->r_extern) {
                if (rp->r_symndx >= MAX_SYMBOLS)
                    printf("sym $%d", rp->r_symndx);
                else
                    printf("\"%s\"",
                           &sspace[symbols[rp->r_symndx].asym.iss]);
            } else
                printf("%s", SECTION_NAME[rp->r_symndx]);
            printf(" %s", RELOC_TYPE[rp->r_type]);
            break;
        }
    }
    printf("\n");
}

#define printf  MyPrintf
#include "d.c"

static void
PrintSection(int i)
{
    bool is_text;
    long pc;
    long word;
    char *s;

    printf("Section %s (size %d, relocs %d):\n", sectionHeader[i].s_name,
           sectionHeader[i].s_size, section[i].relocs);
    is_text = strncmp(sectionHeader[i].s_name, ".text", 5) == 0;

    for (unsigned j = 0, pc = sectionHeader[i].s_vaddr;
         j < section[i].length; j++) {
        word = section[i].data[j];
        if (is_text)
            DumpAscii(word, pc);
        else {
            printf("%08x: %08x  ", pc, word);
            s = (char *) &word;
            for (unsigned k = 0; k < 4; k++)
                if (isprint(s[k]))
                    printf("%c", s[k]);
                else
                    printf(".");
            printf("\t%d", word);
        }
        PrintReloc(pc, i, j);
        pc += 4;
    }
}

int
main(int argc, char *argv[])
{
    char *filename = "a.out";
    FILE *f;
    long l;
/* EXTR filesym; */
    char buf[100];

    if (argc == 2)
        filename = argv[1];
    if ((f = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "out: could not open `%s`.\n", filename);
        perror("out");
        exit(1);
    }
    if (!ReadStruct(f, fileHeader)
          || !ReadStruct(f, aoutHeader)
          || fileHeader.f_magic != MIPSELMAGIC) {
        fprintf(stderr,
                "out: %s is not a MIPS Little-Endian COFF object file.\n",
                filename);
        exit(1);
    }
    if (fileHeader.f_nscns > MAX_SECTIONS) {
        fprintf(stderr, "out: too many COFF sections.\n");
        exit(1);
    }
    for (unsigned i = 0; i < fileHeader.f_nscns; i++) {
        ReadStruct(f, sectionHeader[i]);
        if (sectionHeader[i].s_size > MAX_DATA * sizeof (long)
	      && sectionHeader[i].s_scnptr != 0
              || sectionHeader[i].s_nreloc > MAX_RELOCS) {
            printf("section %s is too big.\n", sectionHeader[i].s_name);
            exit(1);
        }
    }
    for (unsigned i = 0; i < fileHeader.f_nscns; i++) {
        if (sectionHeader[i].s_scnptr != 0) {
            section[i].length = sectionHeader[i].s_size / 4;
            fseek(f, sectionHeader[i].s_scnptr, 0);
            fread(section[i].data, sizeof (long), section[i].length, f);
            section[i].relocs = sectionHeader[i].s_nreloc;
            fseek(f, sectionHeader[i].s_relptr, 0);
            fread(section[i].reloc, sizeof (struct reloc),
                  section[i].relocs, f);
        } else
            section[i].length = 0;
    }
    fseek(f, fileHeader.f_symptr, 0);
    ReadStruct(f, symbolHeader);
    if (symbolHeader.iextMax > MAX_SYMBOLS)
        fprintf(stderr, "Too many symbols to store.\n");
    fseek(f, symbolHeader.cbExtOffset, 0);
    for (unsigned i = 0; i < MAX_SYMBOLS && i < symbolHeader.iextMax; i++)
        ReadStruct(f, symbols[i]);
    if (symbolHeader.issExtMax > MAX_SSPACE) {
        fprintf(stderr, "Too large a string space.\n");
        exit(1);
    }
    fseek(f, symbolHeader.cbSsExtOffset, 0);
    fread(sspace, 1, symbolHeader.issExtMax, f);

    for (unsigned i = 0; i < fileHeader.f_nscns; i++)
        PrintSection(i);

    printf("External Symbols:\nValue\t Type\t\tStorage Class\tName\n");
    for (unsigned i = 0; i < MAX_SYMBOLS && i < symbolHeader.iextMax; i++) {
        SYMR *sym = &symbols[i].asym;
        if (sym->sc == scUndefined)
            MyPrintf("\t ");
        else
            MyPrintf("%08x ", sym->value);
        MyPrintf("%s", SYMBOL_TYPE[sym->st]);
        MyTab(25);
        MyPrintf("%s", STORAGE_CLASS[sym->sc]);
        MyTab(41);
        MyPrintf("%s\n", &sspace[sym->iss]);
    }

    return 0;
}
