// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

// In pch.h
#ifndef PCH_H
#define PCH_H

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

// Ensure this is a function call, not a redefinition
inline void EnableMemoryLeakChecks() {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
}

#endif //PCH_H
