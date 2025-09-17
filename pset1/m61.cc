#include "m61.hh" // https://github.com/cs61/cs61-f25-psets/blob/main/pset1/m61.hh
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cinttypes>
#include <cassert>
#include <vector>

struct allocation_info {
    void* ptr;
    size_t size;
    const char* file;
    int line;
    allocation_info (void* p, size_t s, const char* f, int l)
        : ptr(p), size(s), file(f), line(l) {}
};

static std::vector<allocation_info> active_allocations;
static m61_statistics gstats = {0,0,0,0,0,0,0,0};

struct m61_memory_buffer {
    char* buffer;
    size_t pos = 0;
    size_t size = 8 << 20; /* 8 MiB */

    m61_memory_buffer() {
        void* buf = mmap(nullptr, size,              
        PROT_READ | PROT_WRITE,              
        MAP_ANON | MAP_PRIVATE, -1, 0);
        assert(buf != MAP_FAILED);
        buffer = (char*) buf;
    }
    ~m61_memory_buffer(); {
        munmap(buffer, size);
    }
};

static m61_memory_buffer default_buffer;

/// m61_malloc(sz, file, line)
///    Returns a pointer to `sz` bytes of freshly-allocated dynamic memory.
///    The memory is not initialized. If `sz == 0`, then m61_malloc may
///    return either `nullptr` or a pointer to a unique allocation.
///    The allocation request was made at source code location `file`:`line`.

void* m61_malloc(size_t sz, const char* file, int line) {
    if (sz == 0 || default_buffer.pos +sz > default_buffer.size) {
        ++gstats.nfail;
        gstats.fail_size += sz;
        return nullptr;
    }
    void* ptr = &default_buffer.buffer[default_buffer.pos];
    default_buffer.pos +=sz;

    active_allocations.emplace_back(ptr, sz, file, line);
    
    ++gstats.ntotal; 
    gstats.total_size +=sz;
    ++gstats.nactive;
    gstats.active_size += sz;

    uintptr_t ptr_value = reinterpret_cast<uintptr_t>(ptr);
    if (gstats.heap_min == 0 || ptr_value < gstats.heap_min)
        gstats.heap_min = ptr_value;
    if (ptr_value + sz - 1 > gstats.heap_max)
        gstats.heap_max = ptr_value + sz - 1;

    return ptr;
}

/// m61_free(ptr, file, line)
///    Frees the memory allocation pointed to by `ptr`. If `ptr == nullptr`,
///    does nothing. Otherwise, `ptr` must point to a currently active
///    allocation returned by `m61_malloc`. The free was called at location
///    `file`:`line`.

void m61_free(void* ptr, const char* file, int line) {
    if (!ptr) return;
    for (auto it = active_allocations.begin(); it != active_allocations.end(); ++it) {
        if (it->ptr == ptr) {
            gstats.nactive--;
            gstats.active_size -= it->size;
            active_allocations.erase(it);
            return;
        }
    }
    fprintf(stderr, "Invalid free or double free at %s:%d for pointer %p\n",
            file ? file : "???", line, ptr);
}

/// m61_calloc(count, sz, file, line)
///    Returns a pointer a fresh dynamic memory allocation big enough to
///    hold an array of `count` elements of `sz` bytes each. Returned
///    memory is initialized to zero. The allocation request was at
///    location `file`:`line`. Returns `nullptr` if out of memory; may
///    also return `nullptr` if `count == 0` or `size == 0`.

void* m61_calloc(size_t count, size_t sz, const char* file, int line) {
    // Check for overflow
    if (count && sz > SIZE_MAX / count) {
        ++gstats.nfail;
        gstats.fail_size += count *sz; // This might overflow, but for statistics
        return nullptr;
    }
    size_t total_size = count * sz;
    void* ptr = m61_malloc(total_size, file, line);
    if (ptr) 
        memset(ptr, 0, total_size);
    return ptr;
}

/// m61_get_statistics()
///    Return the current memory statistics.

m61_statistics m61_get_statistics() {
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
}
