#include <cstdlib>
#include <map>
#include <string>
#include <iostream>
#include <cmath>

#define HEAPSTAT_NO_OVERRIDE
#include "heapstat.hh"

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

// display first "digits" many digits of number plus unit (kilo-exabytes)
static int human_readable(char* buf, double num)
{
    size_t snap = 0;
    size_t orig = num;
    int unit = 0;
    while (num >= 1000) {
        num /= 1024;
        unit++;
    }
    int len;
    if (unit && num < 100.0)
        len = snprintf(buf, 8, "%.3g", num);
    else
        len = snprintf(buf, 8, "%d", (int)num);

    unit = "\0kMGTPEZY"[unit];

    if (unit) buf[len++] = unit;
    buf[len++] = 'B';
    buf[len] = 0;

    return len;
}

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
    printf("\n%lu HEAP ALLOCATIONS LEAKED\n", ptrMap.size());

    map<string, _Stats> statsMap;
    for (auto kv : ptrMap) {
        const char* desc = kv.second.desc;
        auto d = statsMap.find(desc);
        if (d == statsMap.end()) { statsMap[desc] = _Stats(); }
        statsMap[desc].sum += kv.second.size;
        statsMap[desc].count++;
        sum += kv.second.size;
    }
    // if (statsMap.empty()) return 0;

    puts("--------------------------------------------------------------");
    puts("      Count |       Size (B) | Location                       ");
    puts("==============================================================");
    for (auto kv : statsMap) {
        string key = kv.first;
        _Stats val = kv.second;
        char valSumH[24] = "                       ";
        format(valSumH, val.sum);
        printf("%11lu | %14s | %s\n", val.count, valSumH, key.c_str());
    }
    puts("--------------------------------------------------------------");
    char strsum[24] = "                       ";
    format(strsum, sum);
    // human_readable(strsum, sum);
    printf("%11lu | %14s | %s\n", ptrMap.size(), strsum, "total");
    // printf("Leaked %s B total", strsum);
    // if (sum >= 1024) printf(" (%luB)", sum);
    puts("");
    return sum;
}

} // extern "C"