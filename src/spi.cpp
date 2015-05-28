/*****************************************************************************************
 * Hand-coded SPI interface between the Raspberry Pi and the B-PIP
 *
 * There is commented out code to use the Pi's driver, but this driver
 * seems to fail most of the time.
 *
 * Written by Bernhard Firner (c) 2014
 ****************************************************************************************/

#include "spi.h"

#include <iostream>

//Send the startup sequence to the B-PIP
bool startupBPIP() {
	//Reset by holding low for 1ms, then go high and stay there
	bcm2835_gpio_fsel(RESET, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_set_pud(RESET, BCM2835_GPIO_PUD_UP);
	bcm2835_gpio_write(RESET, LOW);
        //Go low for 2 seconds
        for (int i = 0; i < 200; ++i) {
	  bcm2835_delayMicroseconds(10000);
        }
	bcm2835_gpio_write(RESET, HIGH);
	//Allow 10 ms to start up
	bcm2835_delayMicroseconds(10000);

	uint8_t working = 0;
	//All transitions should occur in 8ms intervals,
	//but the PIP clock is not a reliable source.
	//Start at 7 ms intervals and go up to 9 ms intervals
	for (int interval = 7000; (0 == working and interval <= 9000); interval += 1000) {

		//Set all of the outputs 
		bcm2835_gpio_fsel(SCLK, BCM2835_GPIO_FSEL_OUTP);
		bcm2835_gpio_fsel(MOSI, BCM2835_GPIO_FSEL_OUTP);
		bcm2835_gpio_fsel(MISO, BCM2835_GPIO_FSEL_OUTP);
		//Clear the output pins and leave them low for two 8ms periods
		bcm2835_gpio_clr(SCLK);
		bcm2835_gpio_clr(MOSI);
		bcm2835_gpio_clr(MISO);
		bcm2835_delayMicroseconds(interval);
		bcm2835_delayMicroseconds(interval);

		//SOMI goes high and hold for 8ms
		bcm2835_gpio_write(MISO, HIGH);
		bcm2835_delayMicroseconds(interval);

		//MISO stays high, SCLK goes high, and hold for 8ms
		bcm2835_gpio_write(SCLK, HIGH);
		bcm2835_delayMicroseconds(interval);

		//MISO stays high, SCLK goes low, and hold for 8ms
		bcm2835_gpio_write(SCLK, LOW);
		bcm2835_delayMicroseconds(interval);

		//MISO goes low and hold for 8ms
		bcm2835_gpio_write(MISO, LOW);
		bcm2835_delayMicroseconds(interval);

		//Look for a success indication from the BPIP
		bcm2835_gpio_fsel(MISO, BCM2835_GPIO_FSEL_INPT);
		bcm2835_delayMicroseconds(interval);
		working = bcm2835_gpio_lev(MISO);
	}

	//Clear the output pins and leave them low for two 8ms periods
	bcm2835_gpio_clr(SCLK);
	bcm2835_gpio_clr(MOSI);
	bcm2835_gpio_clr(MISO);

	//Indicate if we succeeded or failed
	if (0 == working) {
		return false;
	}
	else {
		//Wait for the working pin to drop low
		bcm2835_gpio_fsel(MISO, BCM2835_GPIO_FSEL_INPT);
		while ( bcm2835_gpio_lev(MISO));
		return true;
	}
}

void setupSPI() {
	//Use bcm2835_gpio_fsel(uint8_t pin, uint8_t mode) to set pin modes
	//Modes are BCM2835_GPIO_FSEL_INPT for input
	//Modes are BCM2835_GPIO_FSEL_OUTP for output
	bcm2835_gpio_fsel(SCLK, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(MOSI, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(MISO, BCM2835_GPIO_FSEL_INPT);

	//Clear the two output pins
	bcm2835_gpio_clr(SCLK);
	bcm2835_gpio_clr(MOSI);

	bcm2835_gpio_set_pud(SCLK, BCM2835_GPIO_PUD_DOWN);
	bcm2835_gpio_set_pud(MOSI, BCM2835_GPIO_PUD_DOWN);
	bcm2835_gpio_set_pud(MISO, BCM2835_GPIO_PUD_UP);
	//bcm2835_spi_begin();
	//About 1MHz
	//bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_256);
	//About 244KHz
	//bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_1024);
	//bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_2048);
	//About 30.5kHz
	//bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_8192);
	//16KHz
	//bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_16384);
	//bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
}

void tearDownSPI() {
	//bcm2835_spi_end();
}

//Sends and reads a bit, delaying clock_period/2 milliseconds after writing and again after reading
//Going to change data on falling edges, so read on rising edges
uint8_t exchangeBit(uint8_t value, unsigned int clock_period) {
	//Go low and set value, then wait a period and go high
	//Delay half a clock cycle since we may have just gone high from a previous call
	bcm2835_delayMicroseconds(clock_period / 2);
	bcm2835_gpio_write(SCLK, LOW);
	//Change output on falling edge
	if (0 == value) {
		bcm2835_gpio_write(MOSI, LOW);
	}
	else {
		bcm2835_gpio_write(MOSI, HIGH);
	}
	bcm2835_delayMicroseconds(clock_period / 2);
	bcm2835_gpio_write(SCLK, HIGH);
	//Read data from the slave
	uint8_t bit = bcm2835_gpio_lev(MISO);
	return bit;
	//return 0;
}

uint8_t exchangeByte(uint8_t byte, unsigned int clock_period) {
	uint8_t value = 0;
	for (int i = 7; i >= 0; --i) {
		if (HIGH == exchangeBit(0x1 & (byte >> i), clock_period)) {
			value |= 1 << i;
		}
	}
	return value;
	//return bcm2835_spi_transfer(byte);
}


