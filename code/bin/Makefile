# Use normal `make` for this Makefile.
#
# Makefile for:
#
# `coff2noff`
#     Converts a normal MIPS executable into a Nachos executable.
# `coff2flat`
#     Converts a normal MIPS executable into a Nachos 2 executable
#     (obsolete).
# `disassemble`
#     Disassembles a normal MIPS executable.
#
# Copyright (c) 1992      The Regents of the University of California.
#               2016-2017 Docentes de la Universidad Nacional de Rosario.
# All rights reserved.  See `copyright.h` for copyright notice and
# limitation of liability and disclaimer of warranty provisions.

# If the host is big endian (SPARC, SNAKE, etc.):
# change to (`disassemble` and `coff2flat` do not support big endian yet):
# CFLAGS = -I./ -I../threads -DHOST_IS_BIG_ENDIAN
# all: coff2noff


include ../Makefile.env

CC     = gcc
CFLAGS = -std=c99 -I./ -I../ $(HOST)
LD     = gcc


.PHONY: all clean

all: coff2noff coff2flat disassemble

clean:
	$(RM) *.o coff2noff coff2flat disassemble || true

# Converts a COFF file to Nachos object format.
coff2noff: coff2noff.o
	$(LD) $^ -o $@

# Converts a COFF file to a flat address space (for Nachos version 2).
coff2flat: coff2flat.o
	$(LD) $^ -o $@

# Dis-assembles a COFF file.
disassemble: out.o opstrings.o
	$(LD) $^ -o $@

coff2noff.o: coff.h noff.h
coff2flat.o: coff.h
out.o: out.c d.c coff.h instr.h encode.h extern/reloc.h extern/syms.h
