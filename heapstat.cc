#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cstdint>
#include <limits>

#include "heapstat.h"
#undef new
#undef malloc
#undef realloc
#undef free

static void format(char* buf, size_t num)
{
    const int digits = static_cast<int>(log10(static_cast<double>(num + 1)));
    buf += digits + (digits / 3) + 1;
    *buf-- = 0;
    int i = 0;
    while (num > 1) {
        i++;
        int d = ((size_t)num) % 10;
        num /= 10;
        *buf-- = d + 48;
        if (((i % 3) == 0) && num) *buf-- = '\'';
    }
}

constexpr size_t MAGIC = std::numeric_limits<uint64_t>::max() - 31;

struct _PtrHeader {
    size_t magic, magic2;
    size_t size : 63, visited : 1;
    const char* desc;
    _PtrHeader *next, *prev;
};

static size_t _heapTotal = 0;

extern "C" {

static _PtrHeader* lastHeader = NULL;
static _PtrHeader* startHeader = NULL;

static void addHeader(size_t size, const char* desc, void* ret)
{
    _heapTotal += size;
    _PtrHeader* head = (_PtrHeader*)ret;
    head->size = size;
    head->desc = desc;
    head->magic = MAGIC;
    head->magic2 = MAGIC;
    head->prev = lastHeader;
    if (lastHeader)
        lastHeader->next = head;
    else
        startHeader = head;
    head->next = NULL;

    // update lastHeader only at the end!
    lastHeader = head;
}
void* heapstat_malloc(size_t size, const char* desc)
{
    void* ret = malloc(size + sizeof(_PtrHeader));
    if (ret) { addHeader(size, desc, ret); }
    return (void*)((char*)ret + sizeof(_PtrHeader));
}

void* heapstat_realloc(void* ptr, size_t size, const char* desc)
{
    void* ret = (void*)((char*)ptr - sizeof(_PtrHeader));
    ret = realloc(ret, size + sizeof(_PtrHeader));
    if (ret) { addHeader(size, desc, ret); }
    return (void*)((char*)ret + sizeof(_PtrHeader));
}

void heapstat_free(void* ptr, const char* desc)
{
    if (!ptr) return;
    void* ret = (void*)((char*)ptr - sizeof(_PtrHeader));

    _PtrHeader* head = (_PtrHeader*)ret;
    if (head->magic == MAGIC && head->magic2 == MAGIC) {
        _PtrHeader *next = head->next, *prev = head->prev;
        if (head->prev) head->prev->next = next;
        if (head->next) head->next->prev = prev;
        if (head == lastHeader) lastHeader = prev;
        if (head == startHeader) startHeader = next;
        _heapTotal -= head->size;
        head->magic = head->magic2 = 0;
        free(head);
    } else {
        fprintf(stderr,
            "-- WARNING --------------------------------------\n"
            "freeing unknown pointer %p\n"
            "  at %s\n"
            "-------------------------------------------------\n",
            static_cast<void*>(head), desc);
        free(ptr);
    }
}

size_t heapstat()
{
    size_t gcount = 0;
    if (!startHeader) return 0;
    printf("\nHEAP ALLOCATIONS LEAKED:\n");
    puts("--------------------------------------------------------------");
    puts("      Count |       Size (B) | Location                       ");
    puts("==============================================================");
    for (_PtrHeader* head = startHeader; head; head = head->next) {
        if (head->visited) continue;
        const char* desc = head->desc;

        size_t size = 0, count = 0;
        for (_PtrHeader* inner = head; inner; inner = inner->next) {
            // since strings are compile time uniqd the ptrs must be equal
            if (inner->desc != desc) continue;
            inner->visited = 1;
            size += inner->size;
            count++, gcount++;
        }

        char valSumH[24] = "                       ";
        format(valSumH, size);
        printf("%11zu | %14s | %s\n", count, valSumH, desc);
    }
    for (_PtrHeader* head = startHeader; head; head = head->next)
        head->visited = 0;

    puts("--------------------------------------------------------------");
    char strsum[24] = "                       ";
    format(strsum, _heapTotal);
    printf("%11zu | %14s | total\n\n", gcount, strsum);
    return _heapTotal;
}

} // extern "C"

void* operator new(size_t size, const char* desc)
{
    return heapstat_malloc(size, desc);
}
void* operator new[](size_t size, const char* desc)
{
    return heapstat_malloc(size, desc);
}
void operator delete(void* ptr) throw() { heapstat_free(ptr, "<unknown>"); }
void operator delete[](void* ptr) throw() { heapstat_free(ptr, "<unknown>"); }
void operator delete(void* ptr, const char* desc) throw()
{
  heapstat_free(ptr, desc);
}
void operator delete[](void* ptr, const char* desc) throw()
{
  heapstat_free(ptr, desc);
}
