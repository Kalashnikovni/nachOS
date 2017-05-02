# Copyright (c) 1992      The Regents of the University of California.
#               2016-2017 Docentes de la Universidad Nacional de Rosario.
# All rights reserved.  See `copyright.h` for copyright notice and
# limitation of liability and disclaimer of warranty provisions.

MAKE = make
LPR  = lpr
SH   = bash

all:
	$(MAKE) -C threads depend
	$(MAKE) -C threads all
	$(MAKE) -C userprog depend
	$(MAKE) -C userprog all
	$(MAKE) -C vmem depend
	$(MAKE) -C vmem all
	$(MAKE) -C filesys depend
	$(MAKE) -C filesys all
	$(MAKE) -C network depend
	$(MAKE) -C network all
	$(MAKE) -C bin
	$(MAKE) -C test

# Do not delete executables in `test` in case there is no cross-compiler.
clean:
	$(MAKE) -C bin clean
	$(MAKE) -C test clean
	$(SH) -c "rm -f */{core,nachos,DISK,*.o,swtch.s}"

print:
	$(SH) -c '$(LPR) Makefile* */Makefile                              \
	                 threads/*.h threads/*.hh threads/*.cc threads/*.s \
	                 userprog/*.h userprog/*.hh userprog/*.cc          \
	                 filesys/*.hh filesys/*.cc                         \
	                 network/*.hh network/*.cc                         \
	                 machine/*.hh machine/*.cc                         \
	                 bin/noff.h bin/coff.h bin/coff2noff.c             \
	                 test/*.h test/*.c test/*.s'
