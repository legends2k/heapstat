#include "heapstat.h"
#undef malloc
#undef free
#undef new

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

typedef void* Ptr;
typedef const char* CString;

typedef struct {
    size_t size;
    const char* desc;
} _PtrInfo;
MAKE_DICT(Ptr, _PtrInfo)
static Dict(Ptr, _PtrInfo) _ptrDict[1];

typedef struct {
    size_t count, sum;
} _Stat;
MAKE_DICT(CString, _Stat)
static Dict(CString, _Stat) _statDict[1];

static size_t _heapTotal = 0;

static int _cmpsum(const void* a, const void* b)
{
    // For sorting in descending order of sum
    int ia = *(int*)a;
    int ib = *(int*)b;
    size_t va = Dict_val(_statDict, ia).sum;
    size_t vb = Dict_val(_statDict, ib).sum;
    return va > vb ? -1 : va == vb ? 0 : 1;
}

static void format(char* buf, double num)
{
    int digits = log10(num);
    buf += digits + (digits / 3) + 1;
    *buf-- = 0;
    int i = 0;
    while (num > 1) {
        i++;
        int d = ((size_t)num) % 10;
        num /= 10;
        *buf-- = d + 48;
        if (!(i % 3)) *buf-- = '\'';
    }
}
#ifdef __cplusplus
extern "C" {
#endif

void heapstat__free(void* ptr, const char* desc)
{

    int d = Dict_get(Ptr, _PtrInfo)(_ptrDict, ptr);
    if (d < Dict_end(_ptrDict)) {
        // if (! Dict_val(_ptrDict, d).heap) {
        //     printf("free non-heap pointer at %s\n", desc);
        // } else {
        free(ptr);
        _heapTotal -= Dict_val(_ptrDict, d).size;
        Dict_delete(Ptr, _PtrInfo)(_ptrDict, d);
        // }
    } else {
        puts("\n-- WARNING -----------------------------------------------");
        printf("freeing unknown pointer at %s\n", desc);
        puts("----------------------------------------------------------");
    }
}

void* heapstat__malloc(size_t size, const char* desc)
{
    void* ret = malloc(size);
    if (ret) {
        int stat[1];
        _heapTotal += size;
        _PtrInfo inf = { //
            .size = size,
            .desc = desc
        };
        int p = Dict_put(Ptr, _PtrInfo)(_ptrDict, ret, stat);
        Dict_val(_ptrDict, p) = inf;
    }
    return ret;
}

void heapstat_reset()
{
    Dict_freedata(Ptr, _PtrInfo)(_ptrDict);
    // _PtrInfo blank = {};
    memset(_ptrDict, 0, sizeof(_PtrInfo));
    // *_ptrDict = blank;
}

size_t heapstat()
{
    // int i = 0;
    size_t sum = 0;
    if (!Dict_size(_ptrDict)) {
        puts("0 LEAKS");
        return 0;
    }
    int stat[1];

    Dict_foreach(_ptrDict, Ptr key, _PtrInfo val, {
        int d = Dict_put(CString, _Stat)(_statDict, val.desc, stat);
        if (*stat) {
            Dict_val(_statDict, d).sum = 0;
            Dict_val(_statDict, d).count = 0;
        }
        Dict_val(_statDict, d).sum += val.size;
        Dict_val(_statDict, d).count++;
        sum += val.size;
    });

    int i = 0, *indxs = (int*)malloc(_statDict->size * sizeof(int));
    Dict_foreach(_statDict, CString key, _Stat val, { indxs[i++] = _i_; });
    qsort(indxs, _statDict->size, sizeof(int), _cmpsum);
    printf("\n%d LEAKS, %d LOCATIONS\n", Dict_size(_ptrDict),
        Dict_size(_statDict));

    puts("--------------------------------------------------------------");
    puts("      Count |       Size (B) | Location                       ");
    puts("==============================================================");
    for (i = 0; i < _statDict->size; i++) {
        CString key = _statDict->keys[indxs[i]];
        _Stat val = _statDict->vals[indxs[i]];
        char valsum[24];
        format(valsum, val.sum);
        printf("%11zu | %14s | %s\n", val.count, valsum, key);
    }
    puts("----------------------------------------------------------");
    char strsum[24] = "                       ";
    format(strsum, _heapTotal);
    // human_readable(strsum, sum);
    printf("%11d | %14s | %s\n", Dict_size(_ptrDict), strsum, "total");
    Dict_clear(CString, _Stat)(_statDict);

#define DictMemUsage(K, V, dict)                                               \
    (Dict_nBuckets(dict) * (sizeof(K) + sizeof(V))                             \
        + Dict__flagsSize(Dict_nBuckets(dict)))

    size_t ownUsage = DictMemUsage(Ptr, _PtrInfo, _ptrDict)
        + DictMemUsage(CString, _Stat, _statDict);

    char ownbuf[24];
    format(ownbuf, ownUsage);
    printf("\nHeapstat's own usage is extra: %s B\n  %d pointer buckets (%d "
           "pointers)\n  %d stat buckets (%d stats)\n",
        ownbuf, Dict_nBuckets(_ptrDict), Dict_size(_ptrDict),
        Dict_nBuckets(_statDict), Dict_size(_statDict));

    free(indxs);

    return sum;
#undef DictMemUsage
}

#ifdef __cplusplus
} // extern "C"

void* operator new(size_t size, const char* desc)
{
    return heapstat__malloc(size, desc);
}
void* operator new[](size_t size, const char* desc)
{
    return heapstat__malloc(size, desc);
}
void operator delete(void* ptr) throw() { heapstat_free(ptr); }
void operator delete[](void* ptr) throw() { heapstat_free(ptr); }

#endif
