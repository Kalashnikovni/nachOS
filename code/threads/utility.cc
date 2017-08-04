/// Debugging routines.
///
/// Allows users to control whether to print `DEBUG` statements, based on a
/// command line argument.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "threads/utility.hh"

// Warning: may be you will have problems with `va_start`.
#include <stdarg.h>


/// Controls which `DEBUG` messages are printed.
static const char *enableFlags = NULL;

/// Initialize so that only `DEBUG` messages with a flag in `flagList` will
/// be printed.
///
/// If the flag is `+`, we enable all DEBUG messages.
///
/// * `flagList` is a string of characters for whose `DEBUG` messages are to
///   be enabled.
void
DebugInit(const char *flagList)
{
    enableFlags = flagList;
}

/// Return true if `DEBUG` messages with `flag` are to be printed.
bool
DebugIsEnabled(char flag)
{
    if (enableFlags != NULL)
        return strchr(enableFlags, flag) != 0
               || strchr(enableFlags, '+') != 0;
    else
        return false;
}

/// Print a debug message, if `flag` is enabled.  Like `printf`, only with an
/// extra argument on the front.
void
DEBUG(char flag, const char *format, ...)
{
    if (DebugIsEnabled(flag)) {
        va_list ap;
        // You will get an unused variable message here -- ignore it.
        va_start(ap, format);
        vfprintf(stdout, format, ap);
        va_end(ap);
        fflush(stdout);
    }
}
