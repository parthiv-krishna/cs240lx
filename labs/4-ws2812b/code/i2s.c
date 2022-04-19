#include "i2s.h"

i2s_regs_t *i2s_regs = (i2s_regs_t *)I2S_REGS_BASE;
cm_regs_t *cm_regs = (cm_regs_t *)CM_REGS_BASE;

#define addr(x) ((uint32_t)&(x))

void i2s_init(void) {
    // alt0 has i2s functionality
    gpio_set_function(I2S_PIN_CLK, GPIO_FUNC_ALT0);
    gpio_set_function(I2S_PIN_FS, GPIO_FUNC_ALT0);
    gpio_set_function(I2S_PIN_DIN, GPIO_FUNC_ALT0);
    gpio_set_function(I2S_PIN_DOUT, GPIO_FUNC_ALT0);

    // ensure writes to GPIO are complete since we are going to clock manager
    dev_barrier(); 

    PUT32(addr(cm_regs->pcm_ctrl), CM_REGS_MSB | CM_CTRL_XTAL);
    assert(GET32(addr(cm_regs->pcm_ctrl)) == CM_CTRL_XTAL);

    // See https://elinux.org/BCM2835_datasheet_errata#p107-108_table_6-35
    // and BCM2835 peripheral manual page 105 (section 6.3)
    // I think we can get away with an integer divisor of 4
    PUT32(addr(cm_regs->pcm_div), CM_REGS_MSB | (4 << 12));
    assert(GET32(addr(cm_regs->pcm_div)) == (4 << 12));

    // enable the clock manager
    PUT32(addr(cm_regs->pcm_ctrl), CM_REGS_MSB | CM_CTRL_EN | CM_CTRL_XTAL);
    assert(GET32(addr(cm_regs->pcm_ctrl)) == (CM_CTRL_EN | CM_CTRL_XTAL));

    // ensure writes to clock manager are complete since we are going to i2s
    dev_barrier();

    // set frame sizes: BCM peripherals page 131
    // set frame sync length to 32
    // and frame length to 63 + 1 = 64
    uint32_t mode = 0;
    mode = bits_set(mode, I2S_MODE_FSLEN_LB, I2S_MODE_FSLEN_UB, 32);
    mode = bits_set(mode, I2S_MODE_FLEN_LB, I2S_MODE_FLEN_UB, 63);
    PUT32(addr(i2s_regs->mode), mode);
    assert(GET32(addr(i2s_regs->mode)) == mode);

    // set channel width: BCM peripherals page 132
    // total width:  (CH1WEX* 16) + CH1WID + 8
    // here we have 1*16 + 8 + 8 = 32 bit wide channel (i.e. 32 bits per sample)
    uint32_t rxc = 0;
    rxc = bit_set(rxc, I2S_RXC_CH1EN);
    rxc = bits_set(rxc, I2S_RXC_CH1WID_LB, I2S_RXC_CH1WID_UB, 8);
    rxc = bit_set(rxc, I2S_RXC_CH1WEX);
    PUT32(addr(i2s_regs->rx_cfg), rxc);
    assert(GET32(addr(i2s_regs->rx_cfg)) == rxc);

    uint32_t cs = 0;
    cs = bit_set(cs, I2S_CS_STBY); // disable standby
    cs = bit_set(cs, I2S_CS_RXCLR); // clear receive FIFO
    cs = bit_set(cs, I2S_CS_RXON); // enable receive
    PUT32(addr(i2s_regs->cs), cs);
    cs = GET32(addr(i2s_regs->cs));
    assert(bit_is_on(cs, I2S_CS_STBY));
    assert(bit_is_on(cs, I2S_CS_RXON));

    cs = GET32(addr(i2s_regs->cs));
    cs = bit_set(cs, I2S_CS_EN); // enable i2s
    PUT32(addr(i2s_regs->cs), cs);

    dev_barrier();

    cs = GET32(addr(i2s_regs->cs));
    assert(bit_is_on(cs, I2S_CS_EN));

    dev_barrier();
}

int32_t i2s_read_sample(void) {
    dev_barrier();
    // page 127: RXD indicates that the RX FIFO contains data
    while (bit_is_off(GET32(addr(i2s_regs->cs)), I2S_CS_RXR)) {
        // wait for data to be available
    }
    int32_t sample = GET32(addr(i2s_regs->fifo));
    // only reads, no dev_barrier needed
    return sample;
}