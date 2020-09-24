// ---------------------------------------------------------------------------
// HOW TO USE HEAPSTAT:
// 1. include the header "heapstat.hh"
// 2. add the file "heapstat.cc" to your C++ project and build targets
// 3. Just call heapstat() whenever you want to check for leaks
//    - returns (size_t): total bytes leaked over all allocations so far
// ---------------------------------------------------------------------------
// You need a compiler with C++11 support.
// ---------------------------------------------------------------------------
//                      (C) 2020 The Jet Language Team <sushpa@jetpilots.dev>
// ---------------------------------------------------------------------------

#include <cstdlib>

#define STR(X) #X
#define SPOT(file, line) file ":" STR(line)

extern "C" {
void* heapstat_malloc(size_t size, const char* desc);
void* heapstat_realloc(void* ptr, size_t size, const char* desc);
void heapstat_free(void* ptr, const char* desc);
size_t heapstat();
}

#ifndef HEAPSTAT_DISABLE
#define malloc(sz) heapstat_malloc((sz), SPOT(__FILE__, __LINE__))
#define realloc(ptr, sz) heapstat_realloc((ptr), (sz), SPOT(__FILE__, __LINE__))
#define free(ptr) heapstat_free((ptr), SPOT(__FILE__, __LINE__))
inline void* operator new(size_t size, const char* desc)
{
    return heapstat_malloc(size, desc);
}
inline void* operator new[](size_t size, const char* desc)
{
    return heapstat_malloc(size, desc);
}
void operator delete(void* ptr) noexcept { heapstat_free(ptr, "/"); }
void operator delete[](void* ptr) noexcept { heapstat_free(ptr, "/"); }

#define new new (SPOT(__FILE__, __LINE__))
// #define delete delete (SPOT(__FILE__, __LINE__))
#endif