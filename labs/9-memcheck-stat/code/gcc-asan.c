#include "rpi.h"
#include "ckalloc.h"

void __asan_load1_noabort(unsigned long addr) {
    uint32_t lr;
    asm volatile("mov %0, lr" : "=r"(lr));
    asan_access(addr, 1, 0, lr);
}

void __asan_load2_noabort(unsigned long addr) {
    uint32_t lr;
    asm volatile("mov %0, lr" : "=r"(lr));
    asan_access(addr, 2, 0, lr);
}

void __asan_load4_noabort(unsigned long addr) {
    uint32_t lr;
    asm volatile("mov %0, lr" : "=r"(lr));
    asan_access(addr, 4, 0, lr);
}

void __asan_load8_noabort(unsigned long addr) {
    uint32_t lr;
    asm volatile("mov %0, lr" : "=r"(lr));
    asan_access(addr, 8, 0, lr);
}

void __asan_loadN_noabort(unsigned long addr, size_t sz) {
    uint32_t lr;
    asm volatile("mov %0, lr" : "=r"(lr));
    asan_access(addr, sz, 0, lr);
}

void __asan_store1_noabort(unsigned long addr) {
    uint32_t lr;
    asm volatile("mov %0, lr" : "=r"(lr));
    asan_access(addr, 1, 1, lr);
}

void __asan_store2_noabort(unsigned long addr) {
    uint32_t lr;
    asm volatile("mov %0, lr" : "=r"(lr));
    asan_access(addr, 2, 1, lr);
}

void __asan_store4_noabort(unsigned long addr) {
    uint32_t lr;
    asm volatile("mov %0, lr" : "=r"(lr));
    asan_access(addr, 4, 1, lr);
}

void __asan_store8_noabort(unsigned long addr) {
    uint32_t lr;
    asm volatile("mov %0, lr" : "=r"(lr));
    asan_access(addr, 8, 1, lr);
}

void __asan_storeN_noabort(unsigned long addr, size_t sz) {
    uint32_t lr;
    asm volatile("mov %0, lr" : "=r"(lr));
    asan_access(addr, sz, 1, lr);
}


void __asan_handle_no_return() {
    // empty
}

void __asan_before_dynamic_init(const char* module_name) {
    // empty
}

void __asan_after_dynamic_init() {
    // empty
}
