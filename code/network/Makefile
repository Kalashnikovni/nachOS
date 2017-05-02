# NOTE: this is a GNU Makefile.  You must use GNU Make; other `make`
# implementations may not work.
#
# Makefile for the network assignment.
# Defines set up assuming this assignment is done last.  If not, use the
# “bare bones” defines below.
#
# Copyright (c) 1992      The Regents of the University of California.
#               2016-2017 Docentes de la Universidad Nacional de Rosario.
# All rights reserved.  See `copyright.h` for copyright notice and
# limitation of liability and disclaimer of warranty provisions.


DEFINES      = -DUSER_PROGRAM -DVMEM -DFILESYS_NEEDED -DFILESYS -DNETWORK
INCLUDE_DIRS = -I.. -I../bin -I../filesys -I../vm -I../userprog \
               -I../threads -I../machine
HFILES       = $(THREAD_H) $(USERPROG_H) $(VMEM_H) $(FILESYS_H) $(NETWORK_H)
CFILES       = $(THREAD_C) $(USERPROG_C) $(VMEM_C) $(FILESYS_C) $(NETWORK_C)
C_OFILES     = $(THREAD_O) $(USERPROG_O) $(VMEM_O) $(FILESYS_O) $(NETWORK_O)

# Bare bones version.
#DEFINES      = -DTHREADS -DNETWORK
#INCLUDE_DIRS = -I.. -I../network -I../threads -I../machine
#HFILES       = $(THREAD_H) $(NETWORK_H)
#CFILES       = $(THREAD_C) $(NETWORK_C)
#C_OFILES     = $(THREAD_O) $(NETWORK_O)

include ../Makefile.common
include ../Makefile.env
-include Makefile.depends
