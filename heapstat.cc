#include <cstdlib>
#include <cstdio>
#include <cmath>

#define HEAPSTAT_DISABLE
#include "heapstat.hh"

static void format(char* buf, double num)
{
    int digits = log10(num + 1);
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

struct _PtrHeader {
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
    void* ret = (void*)((char*)ret - sizeof(_PtrHeader));
    ret = realloc(ptr, size);
    if (ret) { addHeader(size, desc, ret); }
    return (void*)((char*)ret + sizeof(_PtrHeader));
}

void heapstat_free(void* ptr)
{
    if (!ptr) return;
    void* ret = (void*)((char*)ptr - sizeof(_PtrHeader));

    _PtrHeader* buf = (_PtrHeader*)ret;
    _PtrHeader *next = buf->next, *prev = buf->prev;
    if (buf->prev) buf->prev->next = next;
    if (buf->next) buf->next->prev = prev;
    _heapTotal -= buf->size;
    free(buf);
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
            count++;
            gcount++;
        }

        char valSumH[24] = "                       ";
        format(valSumH, size);
        printf("%11lu | %14s | %s\n", count, valSumH, desc);
    }
    for (_PtrHeader* head = startHeader; head; head = head->next)
        head->visited = 0;

    puts("--------------------------------------------------------------");
    char strsum[24] = "                       ";
    format(strsum, _heapTotal);
    printf("%11lu | %14s | %s\n", gcount, strsum, "total");
    puts("");
    return _heapTotal;
}

} // extern "C"