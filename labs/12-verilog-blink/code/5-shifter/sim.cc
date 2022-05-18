#include <stdlib.h>
#include "Vshifter.h"
#include "verilated.h"

void check_left(Vshifter* dut, uint32_t in, uint32_t shift) {
    dut->in = in;
    dut->shift = shift;
    dut->right = 0;
    dut->eval();
    printf("check: %d << %d = %d\n", in, shift, dut->out);
    assert(dut->out == in << shift);
}

void check_right(Vshifter* dut, uint32_t in, uint32_t shift) {
    dut->in = in;
    dut->shift = shift;
    dut->right = 1;
    dut->eval();
    printf("check: %d >> %d = %d\n", in, shift, dut->out);
    assert(dut->out == in >> shift);
}

int main() {
    Vshifter* dut = new Vshifter;

    for (int i = 0; i < 32; i++) {
        check_left(dut, 1, i);
        check_right(dut, 1, i);
    }

    for (int i = 0; i < 10; i++) {
        uint32_t in = rand();
        uint32_t shift = rand() % 32;
        check_left(dut, in, shift);
        check_right(dut, in, shift);
    }

    return 0;
}

