# NOTE: this is a GNU Makefile.  You must use GNU Make; other `make`
# implementations may not work.
#
# Makefile for the multiprogramming assignment.
# Defines set up assuming multiprogramming is done before the file system.
# If not, use the “filesystem first” defines below.
#
# Copyright (c) 1992      The Regents of the University of California.
#               2016-2017 Docentes de la Universidad Nacional de Rosario.
# All rights reserved.  See `copyright.h` for copyright notice and
# limitation of liability and disclaimer of warranty provisions.


DEFINES      = -DUSER_PROGRAM -DFILESYS_NEEDED -DFILESYS_STUB \
               -DDFS_TICKS_FIX
INCLUDE_DIRS = -I.. -I../bin -I../filesys -I../threads -I../machine
HFILES       = $(THREAD_H) $(USERPROG_H)
CFILES       = $(THREAD_C) $(USERPROG_C)
C_OFILES     = $(THREAD_O) $(USERPROG_O)

# If filesystem is done first!
#DEFINES      = -DUSER_PROGRAM -DFILESYS_NEEDED -DFILESYS
#INCLUDE_DIRS = -I.. -I../bin -I../filesys -I../threads -I../machine
#HFILES       = $(THREAD_H) $(USERPROG_H) $(FILESYS_H)
#CFILES       = $(THREAD_C) $(USERPROG_C) $(FILESYS_C)
#C_OFILES     = $(THREAD_O) $(USERPROG_O) $(FILESYS_O)

include ../Makefile.common
include ../Makefile.env
-include Makefile.depends
