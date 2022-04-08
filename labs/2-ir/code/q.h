#ifndef Q_H
#define Q_H

#include "ir.h"

#define SIZE 16

// ring buffer queue
typedef struct {
  ir_button_t buf[SIZE];
  unsigned head, tail;
} Q;

int q_size(Q *q) {
    return (q->head - q->tail + SIZE) % SIZE;
}


void q_push(Q *q, ir_button_t b) {
    if (q_size(q) == SIZE) {
        panic("full!");
    }
    q->buf[q->head] = b;
    q->head = (q->head + 1) % SIZE;
}

ir_button_t q_pop(Q *q) {
    ir_button_t b = q->buf[q->tail];
    q->tail = (q->tail + 1) % SIZE;
    return b;
}

void q_init(Q *q) {
    q->head = q->tail = 0;
}


#endif // Q_H