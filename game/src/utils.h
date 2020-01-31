#pragma once

#include <stdio.h>
#include <process.h>

inline void panic() {
    __debugbreak();
    abort();
}