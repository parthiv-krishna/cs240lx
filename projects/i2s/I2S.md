# I2S

The r/pi has a hardware I2S peripheral. The docs for this are a bit of a mess. The key section of the BCM manual is chapter 8. I got single-channel receive working and I think that having multi-channel receive should not be too much extra work. Transmitting should also be pretty doable as you just do an equivalent setup for the TX and then write to the FIFO register instead of reading from it. I think you can have both TX and RX going at the same time. 

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
1. Set the I2S pins (above) all to mode `ALT0` (`0b100`)
2. dev_barrier. I haven't tested without this but we're changing from GPIO to Clock Manager peripheral so it's better to be safe.
3. Configure the Clock Manager to manage the clock for I2S. This is pretty much completely undocumented in the Peripherals manual. Every time you write to a Clock Manager register, you must set the MS byte of the word you write to `0x5A`. Why? It's the password for the clock manager. Not a great password if it's published in all the docs...  
    - Write `0b0001` to lowest 4 bits of the PCM_CTRL register (`0x20101098`). This uses the highest resolution clock we have available, the 19.2 MHz XTAL clock.
    - Setup clock divider in the PCM_DIV register (`0x2010109C`). The meaning of this register is explained in the two errata sections listed in Resources, as well as Section 6.3 of the Peripherals manual. The whole MASH filter thing is a bit too complicated for me to do the math, but just setting the integer part (bits 12 and up) to be 4 seemed to work alright. 
    - Enable I2S clock. Write 1 to bit 5 of PCM_CTRL. Make sure not to clear anything you already set. This will tell the Clock Manager to start outputting a 19.2 MHz / 4 = 4.8 MHz clock. This is faster than ideal (44.1 kHz * 32 bits * 2 channels ~= 2.8 MHz) but I noticed no adverse effects for this visualizer demo. Perhaps for something requiring good audio quality you need to be more careful with this. 
4. dev_barrier. Done with clock manager peripheral, now time for I2S.
5. Configure the I2S peripheral. This is reasonably well-documented in the Peripherals manual. It can be configured in either polling, interrupt, or DMA mode. I just did polling but in a "real" project you'd definitely want interrupt or DMA.
    - Mode register (`0x20203008`). Set FLEN field to 63 and FSLEN field to 32. This means that each frame is 63 + 1 = 64 bits, with each channel being 32 bits. This gives us 32 bits for each channel.
    - Receiver config register (`0x2020300C`). Set STBY bit to disable standby, set RXCLR bit to clear the receive FIFO, and set RXON bit to enable receiver.
    - Control and status register (`0x20203000`). Set the EN bit to enable the I2S peripheral.
6. dev_barrier. I2S should now be constantly sending the two clocks out to any peripherals connected to it, and loading samples into the FIFO located at `0x20203004`. 


## Reading a Sample
1. dev_barrier. I haven't tested without this but you may be switching from another peripheral, and you want a dev_barrier before the first read.
2. Wait for the `RXD` bit in the `CS` register to go high. If it's high there is at least 1 sample available in the FIFO.
3. Read the sample from the `FIFO` register.
4. No dev_barrier needed, we didn't write anything.

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