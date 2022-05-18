#include <stdlib.h>
#include "Vaddsub.h"
#include "verilated.h"

void check_add(Vaddsub* dut, uint32_t a, uint32_t b) {
    dut->a = a;
    dut->b = b;
    dut->sub = 0;
    dut->eval();
    printf("check: %d + %d = %d\n", a, b, dut->sum);
    assert(dut->sum == a + b);
}

void check_sub(Vaddsub* dut, uint32_t a, uint32_t b) {
    dut->a = a;
    dut->b = b;
    dut->sub = 1;
    dut->eval();
    printf("check: %d - %d = %d\n", a, b, dut->sum);
    assert(dut->sum == a - b);
}

int main() {
    Vaddsub* dut = new Vaddsub;

    for (int i = 0; i < 10; i++) {
        uint32_t a = rand();
        uint32_t b = rand();
        check_add(dut, a, b);
        check_sub(dut, a, b);
    }

    check_add(dut, 0, 0);
    check_sub(dut, 0, 0);

    check_add(dut, (uint32_t) -1, 1);
    check_sub(dut, (uint32_t) -1, 1);

    check_add(dut, (uint32_t) -1, (uint32_t) -1);
    check_sub(dut, (uint32_t) -1, (uint32_t) -1);

    return 0;
}

