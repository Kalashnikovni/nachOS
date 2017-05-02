# NOTE: this is a GNU Makefile.  You must use GNU Make; other `make`
# implementations may not work.
#
# Makefile for the threads assignment.  The threads assignment must be done
# first!
#
# Copyright (c) 1992      The Regents of the University of California.
#               2016-2017 Docentes de la Universidad Nacional de Rosario.
# All rights reserved.  See `copyright.h` for copyright notice and
# limitation of liability and disclaimer of warranty provisions.


DEFINES      = -DTHREADS -DDFS_TICKS_FIX
INCLUDE_DIRS = -I.. -I../machine
HFILES       = $(THREAD_H)
CFILES       = $(THREAD_C)
C_OFILES     = $(THREAD_O)

include ../Makefile.common
include ../Makefile.env
-include Makefile.depends
