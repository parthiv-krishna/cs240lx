// #ifndef IR_H
// #define IR_H

#include "rpi.h"

typedef enum {
        IR_INVALID_HEADER = 0xdeadbeef, // we should never get this.
        IR_POWER = 0x3c201de2,
        IR_SOURCE = 0x3c215ca2,
        IR_VOL_UP = 0x3c21dc22,
        IR_VOL_DOWN = 0x3c203dc2,
        IR_MUTE = 0x3c211ce2,
        IR_REPLAY = 0x3c2122dc,
        IR_SEEK_BACK = 0x3c21a25c,
        IR_PLAY_PAUSE = 0x3c20a35c,
        IR_SEEK_FORWARD = 0x3c20639c,
        IR_SOUND_EFFECT = 0x3c21bc42,
        IR_SOUND = 0x3c2102fc,
        IR_BT_POWER = 0x3c21728c,
        IR_ARROW_LEFT = 0x3c201be4,
        IR_SOUND_CTRL = 0x3c20cb34,
        IR_ARROW_RIGHT = 0x3c211ae4,
     } ir_button_t;

const char *ir_button_to_string(ir_button_t val);
int ir_init(int pin);
ir_button_t ir_read_button(int pin);


// #endif // IR_H