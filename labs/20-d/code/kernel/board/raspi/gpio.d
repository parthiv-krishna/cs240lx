module kernel.board.raspi.gpio;

// for mmio.st and mmio.ld
import mmio = kernel.mmio;
// for accessing the MMIO base: device.base
import device = kernel.board.raspi.device;

enum PinType {
    tx = 14,
    rx = 15,
    sda = 2,
    scl = 3,
}

enum FuncType {
    input = 0,
    output = 1,
    alt0 = 4,
    alt1 = 5,
    alt2 = 6,
    alt3 = 7,
    alt4 = 3,
    alt5 = 2,
}

enum base = 0x200000;
enum set  = 0x20001C;
enum clr  = 0x200028;
enum lev  = 0x200034;

void set_func(uint pin, FuncType fn) {
    if (pin >= 32) {
        return;
    }
    
    uint off = (pin % 10) * 3;
    uint* g = cast(uint *)(device.base + base) + (pin / 10);

    uint v = mmio.ld(g);
    v &= ~(0b111 << off);
    v |= fn << off;
    mmio.st(cast(uint*)g, v);
}

void set_output(uint pin) {
    set_func(pin, FuncType.output);
}

void set_input(uint pin) {
    set_func(pin, FuncType.input);
}

void set_on(uint pin) {
    if (pin >= 32) {
        return;
    }
    mmio.st(cast(uint *)(device.base + set), 1 << pin);
}

void set_off(uint pin) {
    if (pin >= 32) {
        return;
    }
    mmio.st(cast(uint *)(device.base + clr), 1 << pin);
}

bool read(uint pin) {
    if (pin >= 32) {
        return false;
    }
    uint lev = mmio.ld(cast(uint *)(device.base + lev));
    return ((lev >> pin) & 1) == 1;
}
