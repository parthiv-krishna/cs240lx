# I2S

The r/pi has a hardware I2S peripheral. The docs for this are a bit of a mess. The key section of the BCM manual is chapter 8. 

## Basic terminology

- I2S: Inter-IC Sound, a serial bus standard for communication between digital audio devices.
- PCM: The format in which audio samples are transmitted on an I2S bus. The docs seem to use PCM and I2S interchangeably. Think of a PCM sample as the "packet" of the I2S "network".

## Hardware Setup
- I2S device. I used https://www.adafruit.com/product/3421 .
- Pins:
    - 18, CLK (aka BCLK). The main clock of the I2S bus (usually 2-4 MHz). The I2S transmitter will provide a new bit on the rising edge of the clock and the I2S receiver will read the bit on the falling edge of the clock. 
    - 19, FS (aka WS aka LRCLK). This signal goes low/right when requesting the left/right channels of audio data respectively. FS -> frame sync, WS -> word select.
    - 20, DIN. This is the input for the pi's I2S receiver. If you have a sample-producing device (e.g. microphone) you'd plug its DOUT into here. 
    - 21, DOUT. This is the output for the pi's I2S transmitter. If you have a sample-consuming device (e.g. audio amplifier) you'd plug its DIN into here. 

## Initializing I2S
Here is the process that worked for me:

## Reading a Sample

## Troubleshooting
I ran into an issue for a bit where I was always reading 0x0 out of the RX FIFO. Without a logic analyzer, it was difficult to determine whether the signals being sent were what I expected. Some test ideas to sanity check if things are sort of working:
- Plug in an LED between CLK and ground. When you enable I2S, does it turn on? CLK should be going high/low very quickly so you should see the LED turn on dimly. 
- Plug in an LED between FS and ground. When you enable I2S, does it turn on? FS should be going high/low a bit less (32x less) quickly than CLK so you should see the LED turn on dimly. 
- Plug in an LED between the microphone DOUT and ground, but leave
everything else plugged in. When you enable I2S, does the LED turn on? The mic should be sending data and pulsing the data line accordingly.

In my case, all 3 tests had the LED on, so it seemed that the hardware I2S was outputting the right clocks and the mic was responding with something. But that something seemed to be 0x0 all the time. 
- Next test: connect r/pin DIN (20) to 3V3. When the pi samples the DIN pin on the falling edges of the clock, it should always see a 1. So the FIFO should contain 0xFFFFFFFF always. 

This ended up working. At this point, I remembered that the SEL pin on the adafruit microphone determines which channel of I2S the mic responds to. I had it tied to ground and as soon as I connected it to 3V3, I started getting real data out!

## Resources
[BCM2835-ARM-Peripherals.pdf](https://datasheets.raspberrypi.com/bcm2835/bcm2835-peripherals.pdf)
- Section 8: PCM/I2S
- Section 6.3: General Purpose GPIO Clocks (sorta... it's missing the parts that we need)

[BCM2835 Peripherals Errata](https://elinux.org/BCM2835_datasheet_errata)
- p107-108 table 6-35: explanation of PCM/I2S clock
- p105 table: explanation of clock divider