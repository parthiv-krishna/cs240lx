#include "rpi.h"
#include "ir.h"
#include "rpi-interrupts.h"
#include "vector-base.h"

#define IR_PIN 21
#define LED_PIN 20

#include "q.h"

Q q;

void interrupt_vector(unsigned pc) {
    if (gpio_event_detected(IR_PIN)) {
        ir_button_t button = ir_read_button(IR_PIN);
        // printk("%s\n", ir_button_to_string(button));
        q_push(&q, button);
        gpio_event_clear(IR_PIN);
    }
}


void notmain() {
    ir_init(IR_PIN);
    q_init(&q);

    extern unsigned int _interrupt_table;

    gpio_int_falling_edge(IR_PIN);

    vector_base_set(&_interrupt_table);

    system_enable_interrupts();

    gpio_set_output(LED_PIN);

    int state = 64;

    while (1) {
        if (timer_get_usec() % 128 > state) {
            gpio_write(LED_PIN, 0);
        } else {
            gpio_write(LED_PIN, 1);
        }

        while (q_size(&q) > 0) {
            ir_button_t b = q_pop(&q);
            printk("%s\n", ir_button_to_string(b));
            if (b == IR_VOL_UP) {
               state += 8;
               if (state > 128) {
                   state = 128;
               }
            }
            if (b == IR_VOL_DOWN) {
                state -= 8;
                if (state < 0) {
                    state = 0;
                }
            }
        }

    }

}