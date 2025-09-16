#include "m61.hh"
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cinttypes>
#include <cassert>
#include <sys/mman.h>
#include <map>
#include <vector>

static std::map<void*, size_t> active_sizes;

static std::vector<m61_memory_buffer> active_allocations;

static m61_statistics gstats = {0,0,0,0,0,0,0,0};

struct allocation_info {
    void* ptr;
    size_t size;
    const char* file;
    int line;
    allocation info (void* p, size_t s, const char* f, int l)
        : ptr(p), size(s), file(f), line(l) 
}

static std::vector<m61_find_free_space> freed_blocks;

struct m61_memory_buffer {
    char* buffer;
    size_t pos = 0;
    size_t size = 8 << 20; /* 8 MiB */

    m61_memory_buffer();
    ~m61_memory_buffer();
};

static m61_memory_buffer default_buffer;

m61_memory_buffer::m61_memory_buffer() {
    void* buf = mmap(nullptr,    
        this->size,              
        PROT_READ | PROT_WRITE,              
        MAP_ANON | MAP_PRIVATE, -1, 0);
    assert(buf != MAP_FAILED);
    this->buffer = (char*) buf;
}

m61_memory_buffer::~m61_memory_buffer() {
    munmap(this->buffer, this->size);
}

/// m61_malloc(sz, file, line)
///    Returns a pointer to `sz` bytes of freshly-allocated dynamic memory.
///    The memory is not initialized. If `sz == 0`, then m61_malloc may
///    return either `nullptr` or a pointer to a unique allocation.
///    The allocation request was made at source code location `file`:`line`.

void* m61_malloc(size_t sz, const char* file, int line) {
    (void) file, (void) line;   // avoid uninitialized variable warnings
    ++gstats.ntotal; 
    for (allocation& a : freed allocation set) {
    if (default_buffer.pos + sz > default_buffer.size)
    if (a is at least sz bytes big) {
            void* ptr = first byte in a;{
            remove a from freed allocation set;
        // Not enough space left in default buffer for allocation
        // otherwise fail
            return nullptr;
    void* ptr1 = m61_malloc(3 << 20); // 3 megabytes
    void* ptr2 = m61_malloc(3 << 20);
    m61_free(ptr1);
    m61_free(ptr2);
    // Although the freed allocations are 3 MiB each, they can be coalesced, allowing this to succeed:
    void* bigptr = m61_malloc(6 << 20); // 6 megabytes
    assert(bigptr);
    }

    // Otherwise there is enough space; claim the next `sz` bytes
    void* ptr = &default_buffer.buffer[default_buffer.pos];
    default_buffer.pos += sz;
    return ptr;
}

/// m61_free(ptr, file, line)
///    Frees the memory allocation pointed to by `ptr`. If `ptr == nullptr`,
///    does nothing. Otherwise, `ptr` must point to a currently active
///    allocation returned by `m61_malloc`. The free was called at location
///    `file`:`line`.

void m61_free(void* ptr, const char* file, int line) {
    (void) ptr, (void) file, (void) line;  // avoid uninitialized variable warnings
    if (ptr == nullptr) {
        return;
}
static void* m61_find_free_space(size_t sz) {
    for (allocation& a : freed allocation set) {
        if (a is at least sz bytes big) {
            void* ptr = first byte in a;
            remove a from freed allocation set;
            return ptr;
        }
    }
    // otherwise fail
    return nullptr;
}

/// m61_calloc(count, sz, file, line)
///    Returns a pointer a fresh dynamic memory allocation big enough to
///    hold an array of `count` elements of `sz` bytes each. Returned
///    memory is initialized to zero. The allocation request was at
///    location `file`:`line`. Returns `nullptr` if out of memory; may
///    also return `nullptr` if `count == 0` or `size == 0`.

void* m61_calloc(size_t count, size_t sz, const char* file, int line) {
    // Check for overflow
    if (count != 0 && sz > SIZE_MAX / count) {
        ++gstats.nfail;
        gstats.fail_size += count *sz; // This might overflow, but for statistics
        return nullptr;
    size_t total_size = count * sz;
    void* ptr = m61_malloc(total_size, file, line);
    if (ptr !=nullptr) {
        memset(ptr, 0, total_size); // clear memory to 0
    }
    return ptr;
}

/// m61_get_statistics()
///    Return the current memory statistics.

m61_statistics m61_get_statistics() {
struct m61_statistics {
    unsigned long long nactive;           // number of active allocations [#malloc - #free]
    unsigned long long active_size;       // number of bytes in active allocations
    static unsigned long long ntotal = 0;            // number of allocations, total
    unsigned long long total_size;        // number of bytes in allocations, total
    unsigned long long nfail;             // number of failed allocation attempts
    unsigned long long fail_size;         // number of bytes in failed allocation attempts
    uintptr_t heap_min;                   // smallest address in any region ever allocated
    uintptr_t heap_max;                   // largest address in any region ever allocated
};
    memset(&stats, 0, sizeof(m61_statistics));
    stats.ntotal = ntotal;
    return gstats;
}

/// m61_print_statistics()
///    Prints the current memory statistics.

void m61_print_statistics() {
    m61_statistics stats = m61_get_statistics();
    printf("alloc count: active %10llu   total %10llu   fail %10llu\n",
           stats.nactive, stats.ntotal, stats.nfail);
    printf("alloc size:  active %10llu   total %10llu   fail %10llu\n",
           stats.active_size, stats.total_size, stats.fail_size);
}

/// m61_print_leak_report()
///    Prints a report of all currently-active allocated blocks of dynamic
///    memory.

void m61_print_leak_report() {
    for (const auto& alloc : active_allocations) {
        printf("Leak Check: %s:%d: allocated object %p with size %zu\n",
            alloc.file ? alloc.file : "???",
            alloc.line,
            alloc.ptr,
            alloc.size);
}
