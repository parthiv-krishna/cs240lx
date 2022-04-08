#include "ir.h"

#define PIN 21

void notmain() {
    ir_init(PIN);
    
    while (1) {
        ir_button_t data = ir_read_button(PIN);
        printk("%s\n", ir_button_to_string(data));
    }

}