#include "pch.h"
#include "utils.h"
#include <stdio.h>
#include <process.h>
#include <windows.h>

void panic() {
    __debugbreak();
    abort();
}

void debug_print(const char* lpFmt, ...)
{
    char dbgmsg[200];
    va_list arglist;

    va_start(arglist, lpFmt);
    auto written = _vsnprintf_s(dbgmsg, sizeof(dbgmsg), lpFmt, arglist);
    va_end(arglist);
    written = std::min(written, (int)std::size(dbgmsg) - 2);
    dbgmsg[written] = '\n';
    dbgmsg[written + 1] = '\0';

    OutputDebugString(dbgmsg);
};