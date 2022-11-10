module blink.main;

// for timer.delay_ms
import timer = kernel.timer;
// for gpio functions
import gpio = kernel.gpio;

enum delay_ms = 500;
enum pin = 21;
enum N = 25;

extern (C) void kmain() {
    gpio.set_output(pin);
    for (int i = 0; i < N; i++) {
        gpio.set_on(pin);
        timer.delay_ms(delay_ms);
        gpio.set_off(pin);
        timer.delay_ms(delay_ms);
    }
}
