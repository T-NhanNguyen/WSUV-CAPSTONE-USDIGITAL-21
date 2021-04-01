# WSUV-CAPSTONE-USDIGITAL-21
 Senior capstone project for WSUV class of 2021

Critical problems with the current PCB design:
	There is an accidental short between RO pin of LTC485 and one of the button that I've overlooked. This means that one button would just be unusable

	The LEDs are soldered in the wrong orientation. I didn't expect the LED to be this small

	Q1 power footprint is incorrect (which we've already addressed)

	Analog pin on the chip was not utilized for the Analog encoder port. (There are pin headers that allow us to connect it to another pin, but the Diodes won't be useable).

	AREF is shorted to ground, which prevents us from using ADC functionality of the MCU