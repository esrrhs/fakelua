#pragma once

#include <stddef.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

namespace fakelua {

static inline void* AllocExecutableMemory(size_t size) {
#ifdef _WIN32
    void* ptr = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    return ptr;
#else
    void* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        return NULL;
    }
    return ptr;
#endif
}

static inline void FreeExecutableMemory(void* ptr, size_t size) {
    if (!ptr) {
        return;
    }
#ifdef _WIN32
    VirtualFree(ptr, 0, MEM_RELEASE);
#else
    munmap(ptr, size);
#endif
}

} // namespace fakelua
