# NOTE: this is a GNU Makefile.  You must use GNU Make; other `make`
# implementations may not work.
#
# Makefile for the file system assignment.
# Defines set up assuming multiprogramming and virtual memory done first.
# If not, use the “bare bones” defines below.
#
# Copyright (c) 1992      The Regents of the University of California.
#               2016-2017 Docentes de la Universidad Nacional de Rosario.
# All rights reserved.  See `copyright.h` for copyright notice and
# limitation of liability and disclaimer of warranty provisions.

DEFINES      = -DTHREADS -DUSER_PROGRAM -DVMEM -DFILESYS_NEEDED -DFILESYS
INCLUDE_DIRS = -I.. -I../bin -I../vm -I../userprog -I../threads -I../machine
HFILES       = $(THREAD_H) $(USERPROG_H) $(VMEM_H) $(FILESYS_H)
CFILES       = $(THREAD_C) $(USERPROG_C) $(VMEM_C) $(FILESYS_C)
C_OFILES     = $(THREAD_O) $(USERPROG_O) $(VMEM_O) $(FILESYS_O)

# Bare bones version.
#DEFINES      = -DTHREADS -DFILESYS_NEEDED -DFILESYS
#INCLUDE_DIRS = -I.. -I../threads -I../machine
#HFILES       = $(THREAD_H) $(FILESYS_H)
#CFILES       = $(THREAD_C) $(FILESYS_C)
#C_OFILES     = $(THREAD_O) $(FILESYS_O)

include ../Makefile.common
include ../Makefile.env
-include Makefile.depends
