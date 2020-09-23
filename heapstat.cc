#include <cstdlib>
#include <map>
#include <string>
#include <iostream>

#define HEAPSTAT_NO_OVERRIDE
#include "heapstat.hh"

struct _PtrInfo {
    size_t size;
    const char* desc;
};

struct _Stats {
    size_t count, sum;
};

using std::map;
using std::string;

static map<void*, _PtrInfo> ptrMap;
static size_t _heapTotal = 0;

extern "C" {

void* heapstat_malloc(size_t size, const char* desc)
{
    void* ret = malloc(size);
    if (ret) {
        _heapTotal += size;
        _PtrInfo info = { //
            .size = size,
            .desc = desc
        };
        ptrMap[ret] = info;
    }
    return ret;
}

void* heapstat_realloc(void* ptr, size_t size, const char* desc)
{
    void* ret = realloc(ptr, size);
    if (ret) {
        auto item = ptrMap.find(ptr);
        if (item != ptrMap.end()) _heapTotal -= item->second.size;
        _heapTotal += size;
        _PtrInfo info = { //
            .size = size,
            .desc = desc
        };
        ptrMap[ret] = info;
    }
    return ret;
}

void heapstat_free(void* ptr, const char* desc)
{
    auto item = ptrMap.find(ptr);
    if (item != ptrMap.end()) {
        free(ptr);
        _heapTotal -= item->second.size;
        ptrMap.erase(item);
    } else if (desc and *desc != '/') {
        printf("\nWARNING\nfreeing unknown or freed pointer %p\n  at %s\n", ptr,
            desc);
    }
}

size_t heapstat()
{
    size_t sum = 0;
    if (ptrMap.empty()) return 0;

    map<string, _Stats> statsMap;

    for (auto kv : ptrMap) {
        const char* desc = kv.second.desc;
        auto d = statsMap.find(desc);
        if (d == statsMap.end()) { statsMap[desc] = _Stats(); }
        statsMap[desc].sum += kv.second.size;
        statsMap[desc].count++;
        sum += kv.second.size;
    }
    if (statsMap.empty()) return 0;

    puts("\nHEAP ALLOCATIONS NOT FREED:");
    puts("--------------------------------------------------------------");
    puts("Allocations | Total Size (B) | Location                       ");
    puts("--------------------------------------------------------------");
    for (auto kv : statsMap) {
        string key = kv.first;
        _Stats val = kv.second;
        printf("%11lu | %14lu | %s\n", val.count, val.sum, key.c_str());
    }
    puts("--------------------------------------------------------------");
    printf("Leaked %lu bytes total\n", sum);
    return sum;
}

} // extern "C"