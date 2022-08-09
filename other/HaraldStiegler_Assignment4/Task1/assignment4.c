/*
This file is based on deployed assignment3_tasks.c
Changes: 
.) Added memory mapping from physical GPIO base address used in bare metal programming into this process address space.
.) Using printf for terminal window output instead of sending temperature value via UART
.) Using a while loop with gettimeofday for waiting. According to web discussion this seems to be a more or less reliable way to achieve us accuracy (when running Raspbian). This is important for the temperature sensor as communication via one wire is defined on a us basis.
*/

//includes from raspberrypi.org/forums/viewtopic.php?t=8476 (shows how to map physical GPIO base address into this process memory)
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>//mmap, maps phyiscal GPIO base address into this process' memory
#include <errno.h>
#include <stdint.h>
#include <unistd.h>//getpagesize 
#include <sys/time.h>//gettimeofday used in waiting loop 
#include <string.h>//strerror (error output) 

//------------------------- Begin of imported UART sample code -----------------------------
//I have only added some comments and moved #defines to beginning
//source: https://github.com/me-no-dev/BareMetalPi/blob/master/uart01/uart01.c

/* The base address of the GPIO peripheral (ARM Physical Address) */
#define GPFSEL1 0x20200004
#define GPSET0  0x2020001C
#define GPCLR0  0x20200028
#define GPPUD       0x20200094
#define GPPUDCLK0   0x20200098

#define AUX_ENABLES     0x20215004
#define AUX_MU_IO_REG   0x20215040
#define AUX_MU_IER_REG  0x20215044
#define AUX_MU_IIR_REG  0x20215048
#define AUX_MU_LCR_REG  0x2021504C
#define AUX_MU_MCR_REG  0x20215050
#define AUX_MU_LSR_REG  0x20215054
#define AUX_MU_MSR_REG  0x20215058
#define AUX_MU_SCRATCH  0x2021505C
#define AUX_MU_CNTL_REG 0x20215060
#define AUX_MU_STAT_REG 0x20215064
#define AUX_MU_BAUD_REG 0x20215068

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

//PUT32/GET32/dummy are implemented in example in assembler, so I re-implemented it here in C. In example doc the author justifies assembler by
//the processor's modes: ARM and thumb. I hope I do not have to take care about these modes and this C simply works. 
//UPDATE: No, it did not work in my first trial, volatile was missing and it is essential here in PUT32/GET32, otherwise gcc optimizes too much (->hang)!
void PUT32 ( unsigned int addr, unsigned int value )
{
	volatile unsigned int * p = (unsigned int*)addr;
	*p=value;
}
unsigned int GET32 ( unsigned int addr )
{
	volatile unsigned int* p = (unsigned int*)addr;
	return *p;
}
 void dummy ( unsigned int d )
 {
	 //I guess this function should be empty.
 }


//GPIO14  TXD0 and TXD1
//GPIO15  RXD0 and RXD1
//alt function 5 for uart1
//alt function 0 for uart0

//((250,000,000/115200)/8)-1 = 270

int notmain( void ) //original code consists of uart init and send; split into two parts: init and send
{
	//uart init part starts here
    unsigned int ra;

    PUT32(AUX_ENABLES,1);
    PUT32(AUX_MU_IER_REG,0);
    PUT32(AUX_MU_CNTL_REG,0);
    PUT32(AUX_MU_LCR_REG,3);
    PUT32(AUX_MU_MCR_REG,0);
    PUT32(AUX_MU_IER_REG,0);
    PUT32(AUX_MU_IIR_REG,0xC6);
    PUT32(AUX_MU_BAUD_REG,270);

    ra=GET32(GPFSEL1);
    ra&=~(7<<12); //gpio14
    ra|=2<<12;    //alt5
    PUT32(GPFSEL1,ra);

    PUT32(GPPUD,0);
    for(ra=0;ra<150;ra++) dummy(ra);
    PUT32(GPPUDCLK0,(1<<14));
    for(ra=0;ra<150;ra++) dummy(ra);
    PUT32(GPPUDCLK0,0);

    PUT32(AUX_MU_CNTL_REG,2);
	//uart init part ends here, this init part is copied to function "uart_init"

	//uart sending part starts here
    ra=0;
    while(1)
    {
        while(1)
        {
            if(GET32(AUX_MU_LSR_REG)&0x20) break;
        }
        PUT32(AUX_MU_IO_REG,0x30+(ra++&7));
    }
	//uart sending part ends here, this sending part is copied to function "uart_send_raw"

    return(0);
}
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
//
// Copyright (c) 2012 David Welch dwelch@dwelch.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//-------------------------------------------------------------------------


//------------------------- end of imported UART sample code -----------------------------



//------------------------- Begin of reused UART sample code -----------------------------


void uart_init( void ) //copied from init part of "notmain" 
{
    unsigned int ra=0;

    PUT32(AUX_ENABLES,1);
    PUT32(AUX_MU_IER_REG,0);
    PUT32(AUX_MU_CNTL_REG,0);
    PUT32(AUX_MU_LCR_REG,3);
    PUT32(AUX_MU_MCR_REG,0);
    PUT32(AUX_MU_IER_REG,0);
    PUT32(AUX_MU_IIR_REG,0xC6);
    PUT32(AUX_MU_BAUD_REG,270);

    ra=GET32(GPFSEL1);
    ra&=~(7<<12); //gpio14
    ra|=2<<12;    //alt5
    PUT32(GPFSEL1,ra);

    PUT32(GPPUD,0);
    for(ra=0;ra<150;ra++) dummy(ra);
    PUT32(GPPUDCLK0,(1<<14));
    for(ra=0;ra<150;ra++) dummy(ra);
    PUT32(GPPUDCLK0,0);

    PUT32(AUX_MU_CNTL_REG,2);

}

void uart_send_raw( unsigned int c ) //copied from send part of "notmain"
{
	while(1)
	{
		if(GET32(AUX_MU_LSR_REG)&0x20) break;
	}
	PUT32(AUX_MU_IO_REG,c);
}

void uart_send_bit( int value )
{
	if ( value<= 0)
		uart_send_raw('0');
	else
		uart_send_raw('1');
}

//My own divide method because processor does not support native division!!!! O man I can not believe it.
unsigned int divide(unsigned int numerator, unsigned int denominator)
{
	unsigned int i = 0;
	unsigned int num = numerator;
	if ( numerator == denominator )
	{
		i = 1;
	}
	else
	{
		while (num>=denominator)
		{
			num = num - denominator;
			i++;
		}
	}
	return i;
}

void uart_send_uint( unsigned int value )
{
	char digit_str[10];//required for reversing digit display (most significant digit first)
	unsigned int tmp = value;
	unsigned int lsd_digit = 0;//least significant digit
	unsigned int to_be_sent = 0;
	unsigned int d1 = 0;
	unsigned int i = 0;
	for (i=0;i<10;i++)
		digit_str[i]=0;
	
	if ( value == 0 )
	{
		uart_send_raw('0');
	}
	else
	{
		tmp = value;
		i=0;
		//I use here Konrad's proposed code, although I think my original one was correct too. Never touch running code :-).
		while (tmp!=0)
		{
			if(tmp>=10)
			{
				d1 = divide(tmp, 10);
				//digit = tmp - 10 * tmp / 10;
				if ( tmp==10 )//cruel workaround for "10"
				{//special case "10"
					lsd_digit = 0;
					to_be_sent = lsd_digit + 48;//ascii offset 48 for digit 0
					digit_str[i]=to_be_sent;
					i++;

					//for output of last digit
					lsd_digit = 1;
					tmp = 0;
				}
				else
				{
					lsd_digit = tmp-10*d1;
					//tmp = tmp / 10;
					tmp = d1;
				}

			}else{
				lsd_digit = tmp;
				tmp = 0;
			}
			to_be_sent = lsd_digit + 48;//ascii offset 48 for digit 0
			digit_str[i]=to_be_sent;
			i++;
		}
		for (;i>0;i--)
		{
			uart_send_raw(digit_str[i-1]);
		}
	}
}

//just for debugging
void uart_send_hex_8bit( unsigned char value )
{
	unsigned char i = 0;
	unsigned char hex = 0;
	unsigned char v = value;
	uart_send_raw('0');
	uart_send_raw('x');
	for (i=8;i>0;)
	{
		hex = v&(0xf<<(i-4));
		hex = hex >> (i-4);
		if ( hex > 9)
			uart_send_raw(hex+55);
		else
			uart_send_raw(hex+48);
		i-=4;
	}
}
//just for debugging
void uart_send_hex_16bit( short value )
{
	unsigned char i = 0;
	unsigned char hex = 0;
	unsigned char v = value;
	uart_send_raw('0');
	uart_send_raw('x');
	for (i=16;i>0;)
	{
		hex = v&(0xf<<(i-4));
		hex = hex >> (i-4);
		if ( hex > 9)
			uart_send_raw(hex+55);
		else
			uart_send_raw(hex+48);
		i-=4;
	}
}
//just for debugging
void uart_send_hex_32bit( unsigned int value )
{
	unsigned int i = 0;
	unsigned int hex = 0;
	unsigned int v = value;
	uart_send_raw('0');
	uart_send_raw('x');
	for (i=32;i>0;)
	{
		hex = v&(0xf<<(i-4));
		hex = hex >> (i-4);
		if ( hex > 9)
			uart_send_raw(hex+55);
		else
			uart_send_raw(hex+48);
		i-=4;
	}
}

void print_celsius( unsigned int value )//renamed and adapted from "uart_send_celsius"
{
	unsigned int minus_sign = 45;//ascii code for '-'
	unsigned int plus_sign = 43;//ascii code for '+'
	unsigned int int_part = 0;
	unsigned int uint_val = 0;
	unsigned int frac_part = 0;
	unsigned int frac_value = 0;
	unsigned int tmp = 0;
	volatile unsigned int is_negative = 0;//I have no idea why this variable has to be volatile, but it seems to be necessary
	int sign_val = value&0xffffffff;//I try to fool compiler -> I only want a simple bit-wise assignment; I do not trust compiler when assigning uint to int
	
	//uart_send_hex_32bit(value);
	
	if ( sign_val < 0 )
	{
		//strategy: split into integer and fraction part and handle them separately
		is_negative = 1;//required for output to subtract fraction part
		uint_val = value;

				
		//uart_send_raw(minus_sign);
		printf("-");
		frac_part = uint_val & 0x0000000f;
		
		sign_val = sign_val >> 4;//get rid of temperature fraction part
		sign_val = sign_val | 0xf0000000;//reset most significant 4 bits for 2-complement lost by previous shift
		sign_val = sign_val * (-1);//get rid of 2-complement, convert to positive integer (-> subtract fraction part now instead of adding it when having a 2-complement!)

		uint_val = sign_val & 0xffffffff;//I try to fool compiler -> I only want a simple bit-wise assignment; I do not trust compiler when assigning int to uint
		
		int_part = uint_val;

		//2-complement: adding fractions "moves" number closer to zero (e.g. -11+0.5=-10.5 -> integer part -11 becomes to -10; due to previous multiplication by -1, I have now +11 but I need +10)
		if ( frac_part>0 )
			int_part = int_part-1;

		
	}
	else
	{
		//uart_send_raw(plus_sign);
		printf("+");

		uint_val = value;
		int_part = uint_val >> 4;
		frac_part = uint_val & 0x0000000f;
	}
	//uart_send_uint(int_part);
	//uart_send_raw('.');//decimal point
	printf("%d.",int_part);
	//fraction handling: 
	tmp = frac_part&0x00000008;
	if (tmp!=0)
		frac_value = frac_value+5000;
	tmp = frac_part&0x00000004;
	if (tmp!=0)
		frac_value=frac_value+2500;
	tmp = frac_part&0x00000002;
	if (tmp!=0)
		frac_value=frac_value+1250;
	tmp = frac_part&0x00000001;
	if (tmp!=0)
		frac_value=frac_value+625;//must be right aligned
	if (is_negative>0)
	{
		tmp = 10000;
		if (frac_value>0)
		{
			frac_value = tmp-frac_value;
		}
		
	}
	if ( frac_value <1000)//insert 0 (instead .625 -> .0625, because "625" are right aligned digits after decimal point)
		//uart_send_raw('0');
		printf("0");
	//uart_send_uint(frac_value);
	printf("%d",frac_value);

	printf("\n");
}


//------------------------- End of reused UART sample code -----------------------------


//------------------------- Begin of timer related code (extracted from Konrad's published test.c) -----------------------------

#define RPI_SYSTIMER_BASE       0x20003000UL

/* The base address of the GPIO peripheral (ARM Physical Address) */
#define GPIO_BASE       0x20200000UL

// define used GPIO pin
//I use gpfsel2 instead of 1 
#define LED_GPFSEL      GPIO_GPFSEL2
//I use 12 instead of 18
#define LED_GPFBIT      12
#define LED_GPSET       GPIO_GPSET0
#define LED_GPCLR       GPIO_GPCLR0
#define LED_GPIO_BIT    16

#define GPIO_GPFSEL0    0
#define GPIO_GPFSEL1    1
#define GPIO_GPFSEL2    2
#define GPIO_GPFSEL3    3
#define GPIO_GPFSEL4    4
#define GPIO_GPFSEL5    5

#define GPIO_GPSET0     7
#define GPIO_GPSET1     8

#define GPIO_GPCLR0     10
#define GPIO_GPCLR1     11

#define GPIO_GPLEV0     13
#define GPIO_GPLEV1     14

#define GPIO_GPEDS0     16
#define GPIO_GPEDS1     17

#define GPIO_GPREN0     19
#define GPIO_GPREN1     20

#define GPIO_GPFEN0     22
#define GPIO_GPFEN1     23

#define GPIO_GPHEN0     25
#define GPIO_GPHEN1     26

#define GPIO_GPLEN0     28
#define GPIO_GPLEN1     29

#define GPIO_GPAREN0    31
#define GPIO_GPAREN1    32

#define GPIO_GPAFEN0    34
#define GPIO_GPAFEN1    35

#define GPIO_GPPUD      37
#define GPIO_GPPUDCLK0  38
#define GPIO_GPPUDCLK1  39

/*
typedef struct {
    volatile uint32_t control_status;
    volatile uint32_t counter_lo;
    volatile uint32_t counter_hi;
    volatile uint32_t compare0;
    volatile uint32_t compare1;
    volatile uint32_t compare2;
    volatile uint32_t compare3;
} rpi_sys_timer_t;*/

//Changing from uint32_t to unsigned int makes <stdint.h> obsolete. Who knows what is included from there. I am very sceptical.
typedef struct {
    volatile unsigned int control_status;
    volatile unsigned int counter_lo;
    volatile unsigned int counter_hi;
    volatile unsigned int compare0;
    volatile unsigned int compare1;
    volatile unsigned int compare2;
    volatile unsigned int compare3;
} rpi_sys_timer_t;
static rpi_sys_timer_t * rpiSystemTimer;// = (rpi_sys_timer_t *)RPI_SYSTIMER_BASE;


void RPI_WaitMicroSeconds( unsigned int us )
{
    /*
	This implementation is not used any more, because it requires a mmap, which fails. According to recommendations on web I use timing via gettimeofday for us accuracy.

    printf("RPI_WaitMicroSeconds call begins...\n");//just for debugging, look if mmap works
    unsigned int ts = rpiSystemTimer->counter_lo;
    unsigned int waitUntil = ts+us;
    while( rpiSystemTimer->counter_lo < waitUntil )
    {
         Do nothing. 
    }
    printf("RPI_WaitMicroSeconds call ends\n");//just for debugging, look if mmap works
   */

   //copied from raspberrypi.org/forums/viewtopic.php?t=17688
   struct timeval now, pulse;
   int cycles, micros, delay_micros;
   cycles=0;
   gettimeofday(&pulse,NULL);
   micros=0;
   while (micros<us)
  {
     ++cycles;
     gettimeofday(&now,NULL);
     if (now.tv_sec>pulse.tv_sec) micros=1000000L; else micros=0;
     micros=micros+(now.tv_usec-pulse.tv_usec);
  }
}



//------------------------- End of timer related code (extracted from Konrad's published test.c) -----------------------------



//------------------------- Begin of One Wire related code -----------------------------
/** GPIO Register set */
volatile unsigned int* gpio;

//Left from previous assignment, used for debugging.
void turn_off()
{
	gpio[LED_GPCLR] = (1 << LED_GPIO_BIT);
}
//Left from previous assignment, used for debugging.
void turn_on()
{
	gpio[LED_GPSET] = (1 << LED_GPIO_BIT);
}

void set_gpio_pin_to_output()
{
	//gpio pin 4 is used for output
	gpio[GPIO_GPFSEL0] |= (1 << 12);
}

void set_gpio_pin_to_input()
{	//gpio pin 4 is used for input
	unsigned int oldState = gpio[GPIO_GPFSEL0];//read old register value//debug value: 849492->294948
	//uart_send_hex_32bit(oldState);
	unsigned int inputBitMask = 0xffff8fff;// binary mask: 11 11/1 111 /111 1/11 11/1 000 /111 1/11 11/1 111, set gpio pin 4 to input state (bits 14-12 set to 0)
	//uart_send_raw(' ');
	//uart_send_hex_32bit(excludeBitMask);
	unsigned int newState = oldState & inputBitMask;//update only register values for gpio pin 4, leave others untouched
	//uart_send_raw(' ');
	//uart_send_hex(newState);
	gpio[GPIO_GPFSEL0] = newState;//write input bit pattern back to GPIO_GPFSEL0 register
}

//Read GPIO pin 4, is it low or high?
unsigned int is_gpio_pin_high()
{
	unsigned int gpio_pin_4_bit_mask=(1<<4);
	//uart_send_hex_32bit(gpio_pin_4_bit_mask);
	unsigned int current_bit = gpio[GPIO_GPLEV0]&gpio_pin_4_bit_mask;
	//uart_send_raw(' ');

	if ( current_bit >0 )
	{
		//uart_send_raw('1');
		return 1;
	}
	else
	{
		//uart_send_raw('0');
		return 0;
	}
	//return current_bit;
}

int set_gpio_pin_high()
{
	gpio[LED_GPSET] |= (1 << 4);
}

int set_gpio_pin_low()
{
	gpio[LED_GPCLR] |= (1 << 4);
}

void one_wire_write_bit_1()
{
	/*
	Inspired by https://github.com/rsta2/circle, addon/OneWire
	noInterrupts();
	DIRECT_WRITE_LOW(reg, mask);
	DIRECT_MODE_OUTPUT(reg, mask);	// drive output low
	delayMicroseconds(10);
	DIRECT_WRITE_HIGH(reg, mask);	// drive output high
	interrupts();
	delayMicroseconds(55);*/
	
	//'Circle' first sets pin to low, then to output state. I would have done this in the reverse way, but if it works, you are right, and it seems to work.
	set_gpio_pin_low();
	set_gpio_pin_to_output();
	RPI_WaitMicroSeconds(10);//circle
	//RPI_WaitMicroSeconds(1);
	set_gpio_pin_high();
	RPI_WaitMicroSeconds(55);//circe
	//RPI_WaitMicroSeconds(59);
}

//First naive implementation of timing diagram for Write '0' in Figure 16, page 16 of DS18B20.pdf
/*
void one_wire_write_bit_1()
{
	set_gpio_pin_to_output();
	set_gpio_pin_low();
	RPI_WaitMicroSeconds(1);
	set_gpio_pin_high();
	RPI_WaitMicroSeconds(59);//in total 60us
}*/

void one_wire_write_bit_0()
{
	/*
	Inspired by https://github.com/rsta2/circle, addon/OneWire
	noInterrupts();
	DIRECT_WRITE_LOW(reg, mask);
	DIRECT_MODE_OUTPUT(reg, mask);	// drive output low
	delayMicroseconds(65);
	DIRECT_WRITE_HIGH(reg, mask);	// drive output high
	interrupts();
	delayMicroseconds(5);
	*/
	//'Circle' first sets pin to low, then to output state. I would have done this in the reverse way.
	set_gpio_pin_low();
	set_gpio_pin_to_output();
	RPI_WaitMicroSeconds(65);//circle
	//RPI_WaitMicroSeconds(100);
	set_gpio_pin_high();
	//RPI_WaitMicroSeconds(5);//Trec
}

//First naive implementation of timing diagram for Write '0' in Figure 16, page 16 of DS18B20.pdf
/*void one_wire_write_bit_0()
{
	set_gpio_pin_to_output();
	set_gpio_pin_low();
	RPI_WaitMicroSeconds(100);//something between 60 < Tx'0' < 120 us
}
*/

unsigned int one_wire_read_bit()
{
//Inspired by https://github.com/rsta2/circle, addon/OneWire
/*
	noInterrupts();
	DIRECT_MODE_OUTPUT(reg, mask);
	DIRECT_WRITE_LOW(reg, mask);
	delayMicroseconds(3);
	DIRECT_MODE_INPUT(reg, mask);	// let pin float, pull up will raise
	delayMicroseconds(10);
	r = DIRECT_READ(reg, mask);
	interrupts();
	delayMicroseconds(53);
	return r;
	*/
	//My naive implementation is pretty similar.
	unsigned int result = 0;
	set_gpio_pin_to_output();
	set_gpio_pin_low();
	RPI_WaitMicroSeconds(3);//circle
	set_gpio_pin_to_input();
	RPI_WaitMicroSeconds(10);//circle
	//RPI_WaitMicroSeconds(5);
	result = is_gpio_pin_high();
	RPI_WaitMicroSeconds(53);//circle
	//RPI_WaitMicroSeconds(45);
	
	//RPI_WaitMicroSeconds(5);//Trec
	return result;
}

//My first naive implementation of timing diagram for Read '0'/'1' in Figure 16, page 16 of DS18B20.pdf
/*
int one_wire_read_bit()
{
	volatile int one_wire_bit = 0;
	set_gpio_pin_to_output();
	set_gpio_pin_low();
	RPI_WaitMicroSeconds(3);
	set_gpio_pin_to_input();
	RPI_WaitMicroSeconds(8);
	one_wire_bit=is_gpio_pin_high();//"MASTER SAMPLES"
	RPI_WaitMicroSeconds(4);
	RPI_WaitMicroSeconds(45);
	return one_wire_bit;
}*/

//Inspired by https://github.com/rsta2/circle, addon/OneWire
void one_wire_write_byte(unsigned char value, unsigned char power /* = 0 */)
{
	/*
    uint8_t bitMask;

    for (bitMask = 0x01; bitMask; bitMask <<= 1) {
	OneWire::write_bit( (bitMask & v)?1:0);
    }
    if ( !power) {
		noInterrupts();
		DIRECT_MODE_INPUT(baseReg, bitmask);
		DIRECT_WRITE_LOW(baseReg, bitmask);
		interrupts();
    }
	*/	
	unsigned char bit_mask = 0;
	unsigned char digit_bit = 0;
	unsigned int i;
	for (i=0;i<8;i++)
	{
		bit_mask = (1 << i);
		//uart_send_raw('c');
		//uart_send_hex_32bit(bit_mask);
		digit_bit = value&bit_mask;
		//uart_send_raw('w');
		//uart_send_hex_8bit(digit_bit);
		if ( digit_bit > 0)
			one_wire_write_bit_1();
		else
			one_wire_write_bit_0();
	}
	/*
	This comes from 'circle', it seems to be optional (more robust?).
	if ( power == 0 )
	{
		set_gpio_pin_to_input();
		set_gpio_pin_low();
	}*/
}

//My first naive implementation of sending a byte	
/*void one_wire_send_byte(char value)
{
	unsigned char bitmask = 0;
	unsigned char wanted_bit = 0;
	int i;
	for (i=0;i<8;i++)
	{
		bitmask = (1 << i);
		wanted_bit = value&bitmask;
		if ( wanted_bit > 0)
			one_wire_write_bit_1();
		else
			one_wire_write_bit_0();
	}
}
*/


//Inspired by https://github.com/rsta2/circle, addon/OneWire, but my naive implementation seems to be correct too.
unsigned char one_wire_read_byte()
{
    /*uint8_t bitMask;
    uint8_t r = 0;

    for (bitMask = 0x01; bitMask; bitMask <<= 1) {
	if ( OneWire::read_bit()) r |= bitMask;
    }
    return r;*/
	
	unsigned char read_bit = 0;
	unsigned char read_byte = 0;
	int i;
	for (i=0;i<8;i++)
	{
		read_bit = one_wire_read_bit();
		if ( read_bit!=0)
		{
			read_byte = read_byte | (1<<i);
		}
	}
	return read_byte;
}

//Naive implementation of reading a byte
/*char one_wire_read_byte()
{
	char read_bit = 0;
	char read_byte = 0;
	int i;
	for (i=0;i<8;i++)
	{
		read_bit = one_wire_read_bit();
		if ( read_bit!=0)
		{
			read_byte = read_byte | (1<<i);
		}
	}
	return read_byte;
}
*/

void set_gpio_pin_pullModeOff()
{
	//Inspired by https://github.com/rsta2/circle, addon/OneWire
	/*uintptr nClkReg = ARM_GPIO_GPPUDCLK0 + m_nRegOffset;

	assert (Mode <= 2);
	write32 (ARM_GPIO_GPPUD, Mode);
	CTimer::SimpleusDelay (5);		// 1us should be enough, but to be sure
	write32 (nClkReg, m_nRegMask);
	CTimer::SimpleusDelay (5);		// 1us should be enough, but to be sure
	write32 (ARM_GPIO_GPPUD, 0);
	write32 (nClkReg, 0);*/

	//I have no naive implementation for that. I would never have thought of using those registers for initialization. But I trust the "circle" project for doing the right thing.
		gpio[GPIO_GPPUD]=0;
		RPI_WaitMicroSeconds(5);
		gpio[GPIO_GPPUDCLK0]=(1<<4);
		RPI_WaitMicroSeconds(5);
		gpio[GPIO_GPPUD]=0;
		gpio[GPIO_GPPUDCLK0]=0;
}



//------------------------- End of One Wire related code -----------------------------


//returns 0 if something seems to be found on pin 4 by one-write, otherwise != 0
unsigned int temp_sensor_reset()
{
/*
	//Inspired by https://github.com/rsta2/circle, addon/OneWire
	
	DIRECT_MODE_INPUT(reg, mask);
	interrupts();
	// wait until the wire is high... just in case
	do {
		if (--retries == 0) return 0;
		delayMicroseconds(2);
	} while ( !DIRECT_READ(reg, mask));

	noInterrupts();
	DIRECT_WRITE_LOW(reg, mask);
	DIRECT_MODE_OUTPUT(reg, mask);	// drive output low
	interrupts();
	delayMicroseconds(480);
	noInterrupts();
	DIRECT_MODE_INPUT(reg, mask);	// allow it to float
	delayMicroseconds(70);
	r = !DIRECT_READ(reg, mask);
	interrupts();
	delayMicroseconds(410);*/

	unsigned int pin_state_temp = 0;
	unsigned int retries = 125;

	//pin initialization -> set pull mode to "GPIOPullModeOff" (=0?)
	/*set_gpio_pin_to_input();
	set_gpio_pin_pullModeOff();
	set_gpio_pin_to_input();
	do {
		if (--retries == 0)
		{
			return 0;	
		}
		RPI_WaitMicroSeconds(2);
		pin_state_temp = is_gpio_pin_high();
	} while ( pin_state_temp==0 );*/
	//uart_send_raw('r');
	//uart_send_uint(retries);
	//uart_send_raw(' ');
	set_gpio_pin_low();
	set_gpio_pin_to_output();
	RPI_WaitMicroSeconds(480);
	set_gpio_pin_to_input();
	RPI_WaitMicroSeconds(70);
	
	//pin_state_temp = 0;
	//uart_send_raw('b');
	//uart_send_uint(pin_state_temp);

	pin_state_temp = !is_gpio_pin_high();
	//uart_send_raw('a');
	//uart_send_uint(pin_state_temp);
	RPI_WaitMicroSeconds(410);

	/* first naive implementation
	unsigned int sample_count = 0;
	unsigned int pin_state = 0;
	//turn_on();
	set_gpio_pin_to_output();
	set_gpio_pin_low();
	RPI_WaitMicroSeconds(480);
	set_gpio_pin_high();
	set_gpio_pin_to_input();
	RPI_WaitMicroSeconds(60);
	//uart_send_raw('x');
	//in total 480us
	while (sample_count<12)
	{
		//uart_send_raw(0x30+sample_count);
		//uart_send_raw(':');
		pin_state += is_gpio_pin_high();
		//uart_send_bit(pin_state);
		//uart_send_raw(' ');
		sample_count=sample_count+1;
		//uart_send_raw('\r');
		//uart_send_raw('\n');
		RPI_WaitMicroSeconds(40);
	}
	//uart_send_raw('y');
	//turn_off();
	*/
	return pin_state_temp;
}

//Copied from https://github.com/rsta2/circle, addon/OneWire
unsigned char crc8(unsigned char *addr, unsigned char length)
{
	unsigned char crc = 0;
	unsigned char len = length;
	unsigned char i = 0;
	unsigned char mix = 0;
	unsigned char inbyte = 0;
	while (len--) 
	{
		inbyte = *addr++;
		for (i = 8; i; i--) {
			mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix) crc ^= 0x8C;
			inbyte >>= 1;
		}
	}
	return crc;
}

unsigned int adjust_to_resolution(unsigned int raw_temperature, unsigned char config)
{
	/*
	Copied from https://github.com/rsta2/circle, addon/OneWire
	
		u8 ucConfig = Data[4] & 0x60;

		// at lower res, the low bits are undefined, so let's zero them
		if (ucConfig == 0x00)
		{
			nRaw = nRaw & ~7;	// 9 bit resolution, 93.75 ms
		}
		else if (ucConfig == 0x20)
		{
			nRaw = nRaw & ~3;	// 10 bit, 187.5 ms
		}
		else if (ucConfig == 0x40)
		{
			nRaw = nRaw & ~1;	// 11 bit, 375 ms
		}
*/		
	unsigned char resolution =  config & 0x60;//mask bit 5 and 6, they indicate the resolution, look at Figure 10 in DS18B20.pdf
	unsigned int temperature = raw_temperature;
	if ( resolution == 0x00 )//9 bit resolution
		temperature = temperature & ~7;//set lowest three bits to zero
	if ( resolution == 0x20 )//10 bit resolution
		temperature = temperature & ~3;//set lowest two bits to zero
	if ( resolution == 0x40 )//11 bit resolution
		temperature = temperature & ~1;//set lowest bit to zero
	return temperature;
}

unsigned int read_from_temp_sensor_in_celsius()
{
	unsigned int pin_state = 0;
	unsigned char data[9];
	unsigned int raw_temperature = 0;
	unsigned int temperature = 0;
	unsigned int celsius = 0;
	unsigned int i;
	unsigned int sign_bit = 0;
	unsigned int tmp = 0;

	//while (1)//just for debugging
	{
		//uart_send_raw('z');
		pin_state = temp_sensor_reset(); 
		if ( pin_state == 0 )
			return 0xFFFF0000;//something has gone wrong, no slave connected to bus: this value is an invalid (because uppermost 2 bytes are not used by temperature)
		//uart_send_uint(pin_state);
		//uart_send_raw('x');
		//RPI_WaitMicroSeconds(1000000);
	}
	one_wire_write_byte(0xCC,0);//skip ROM; Following command is a "broadcast" to all slaves (I assume there is only one slave)
	one_wire_write_byte(0x44,0);//T -> convert temperature
	
	RPI_WaitMicroSeconds(1000000);//at highest resolution (12bit) temperature conversion requires 750ms according to manual; to be on safe side I use a second here
	
	temp_sensor_reset();
	one_wire_write_byte(0xCC,0);//skip ROM; Following command is a "broadcast" to all slaves (I assume there is only one slave)
	one_wire_write_byte(0xBE,0);//read scratch pad (=stored temperatur in DS18B20)
	
	for (i = 0; i < 9; i++)
	{
		data[i] = one_wire_read_byte();
	}
	unsigned char crc = crc8(data,8);
	if ( crc == data[8])
	{
		/*for (i=0;i<9;i++)
		{
			uart_send_raw('a');
			uart_send_hex_8bit(data[i]);
			uart_send_raw('b');
		}*/
		//Testing negative temperatures: table 1 in DS18B20 manual
		//data[1]=0xff;//-10.125 degree celsius
		//data[0]=0x5e;
		//data[1]=0xfe;//-25.0625 degree celsius
		//data[0]=0x6f;
		//data[1]=0xfc;//-55 degree celsius
		//data[0]=0x90;
		//data[1]=0x01;//official reference sample value when running Raspberry OS: t=22250
		//data[0]=0x67;
		//raw_temperature = (data[1]<<8) | data[0];//| operator does not work, compiler too stupid
		raw_temperature = data[1];
		raw_temperature = raw_temperature << 8;
		raw_temperature = raw_temperature + data[0];
		//uart_send_hex_32bit(raw_temperature);
		temperature = adjust_to_resolution( raw_temperature, data[4]);

		/*uart_send_raw('d');
		uart_send_hex_8bit(0x0);
		uart_send_raw(' ');
		
		uart_send_raw('0');
		uart_send_hex_8bit(data[0]);
		uart_send_raw(' ');

		uart_send_raw('1');
		uart_send_hex_8bit(data[1]);
		uart_send_raw(' ');
		
		uart_send_raw('t');
		uart_send_hex_32bit(raw_temperature);
		uart_send_raw(' ');*/
		
		//uart_send_hex_32bit(celsius);
		sign_bit = data[1]&0x80;//probe most significant bit (=sign)
		
		//Originally (in 'circle') temperature is converted to celsius here. But all output is shifted to "uart_send_celsius".
		if ( sign_bit==0)
		{
			celsius = temperature;
			//uart_send_raw('r');
			//uart_send_hex_32bit(celsius);
			//uart_send_raw(' ');
		}
		else
		{
			//2-complement, negative value
			celsius = temperature;
			celsius = celsius | (0xffff0000);//preserve 2-complement in 4-byte integer value as data[1] and data[0] are single byte each;
		}
	}
	return celsius;
}

/** Main function - we'll never return from here */
int main(void)
{
	unsigned int celsius = 0;
	
	//notmain();//works, digits 0-7 are output over uart
    	//gpio = (unsigned int*)GPIO_BASE;//bare metal way of getting a pointer to GPIO
	
	//get GPIO base adress, copied from web, look above
	int fd_gpio;
	int fd_timer;
	//printf("open /dev/mem for gpio\n");
	fd_gpio=open("/dev/mem", O_RDWR | O_SYNC );
	if  (fd_gpio<0)
	{
		printf("Unable to open /dev/mem: %s\n",strerror(errno));
		return -1;
	}
	//printf("mmap for gpio\n");
	gpio=(unsigned int*)mmap(0,getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED, fd_gpio, GPIO_BASE);


	//Left from blinking assignment 1.
    /* Write 1 to the GPIO24 init nibble in the Function Select 2 GPIO
       peripheral register to enable GPIO24 as an output */
    	
	//printf("GPIO mmap address access..");
	gpio[LED_GPFSEL] |= (1 << LED_GPFBIT);
	//printf("OK!\n");

	//uart_init();
	//while (1)
	{
		celsius = read_from_temp_sensor_in_celsius();
		if ( celsius != 0xFFFF0000 ) //DS18B20 here
		{	
			print_celsius(celsius);
			//uart_send_celsius(celsius);
			//uart_send_hex_32bit(celsius);
		
			//uart_send_raw(0x0D);//carriage return
			//uart_send_raw(0x0A);//new line
		}
	}
	return 0;
}
