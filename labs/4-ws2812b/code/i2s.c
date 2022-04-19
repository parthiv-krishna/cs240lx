#include "i2s.h"


// Set the EN bit to enable the PCM block. Set all operational values to define the frame and
// channel settings. Assert RXCLR and/or TXCLR wait for 2 PCM clocks to ensure the
// FIFOs are reset. The SYNC bit can be used to determine when 2 clocks have passed.
// Set RXTHR/TXTHR to determine the FIFO thresholds.

// If transmitting, ensure that sufficient sample words have been written to PCMFIFO
// before transmission is started. Set TXON and/or RXON to begin operation. Poll
// TXW writing sample words to PCMFIFO and RXR reading sample words from
// PCMFIFO until all data is transferred.


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

    // See https://elinux.org/BCM2835_datasheet_errata#p107-108_table_6-35
    // and BCM2835 peripheral manual page 105 (section 6.3)
    // I think we can get away with an integer divisor of 4
    PUT32(addr(cm_regs->pcm_div), CM_REGS_MSB | (4 << 12));

    // enable the clock manager
    PUT32(addr(cm_regs->pcm_ctrl), CM_REGS_MSB | CM_CTRL_EN | CM_CTRL_XTAL);

    // ensure writes to clock manager are complete since we are going to i2s
    dev_barrier();

    // set frame sizes: BCM peripherals page 131
    // set frame sync length to 32
    // and frame length to 63 + 1 = 64
    uint32_t mode = 0;
    mode = bits_set(mode, I2S_MODE_FSLEN_LB, I2S_MODE_FSLEN_UB, 32);
    mode = bits_set(mode, I2S_MODE_FLEN_LB, I2S_MODE_FLEN_UB, 63);
    PUT32(addr(i2s_regs->mode), mode);

    // set channel width: BCM peripherals page 132
    // total width:  (CH1WEX* 16) + CH1WID + 8
    // here we have 1*16 + 8 + 8 = 32 bit wide channel (i.e. 32 bits per sample)
    uint32_t rxc = 0;
    rxc = bit_set(rxc, I2S_RXC_CH1EN);
    rxc = bits_set(rxc, I2S_RXC_CH1WID_LB, I2S_RXC_CH1WID_UB, 8);
    rxc = bit_set(rxc, I2S_RXC_CH1WEX);
    PUT32(addr(i2s_regs->rx_cfg), rxc);

    uint32_t cs = 0;
    cs = bit_set(cs, I2S_CS_STBY); // disable standby
    cs = bit_set(cs, I2S_CS_RXCLR); // clear receive FIFO
    cs = bit_set(cs, I2S_CS_RXON); // enable receive
    PUT32(addr(i2s_regs->cs), cs);

    cs = GET32(addr(i2s_regs->cs));
    bit_set(cs, I2S_CS_EN); // enable i2s
    PUT32(addr(i2s_regs->cs), cs);

    dev_barrier();

    cs = GET32(addr(i2s_regs->cs));
    demand(bit_is_on(cs, I2S_CS_EN), "i2s not enabled %x", cs);

    dev_barrier();

}