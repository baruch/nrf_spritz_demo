#include <stdint.h>
#include <stdio.h>

#include "gpio.h"
#include "delay.h"
#include "uart.h"
#include "interrupt.h"
#include "spritz.h"

#define XBUFLEN 32
#define RBUFLEN 4

/* You might want to specify idata, pdata or idata for the buffers */
static unsigned char __pdata rbuf[RBUFLEN], xbuf[XBUFLEN];
static unsigned char rcnt, xcnt, rpos, xpos;
static __bit busy;

void ser_init(void)
{
   rcnt = xcnt = rpos = xpos = 0;  /* init buffers */
   busy = 0;
   
	// Setup UART pins
	gpio_pin_configure(GPIO_PIN_ID_FUNC_RXD,
			GPIO_PIN_CONFIG_OPTION_DIR_INPUT |
			GPIO_PIN_CONFIG_OPTION_PIN_MODE_INPUT_BUFFER_ON_NO_RESISTORS);

	gpio_pin_configure(GPIO_PIN_ID_FUNC_TXD,
			GPIO_PIN_CONFIG_OPTION_DIR_OUTPUT |
			GPIO_PIN_CONFIG_OPTION_OUTPUT_VAL_SET |
			GPIO_PIN_CONFIG_OPTION_PIN_MODE_OUTPUT_BUFFER_NORMAL_DRIVE_STRENGTH);

	uart_configure_8_n_1_38400();
	uart_rx_disable();
	interrupt_control_uart_enable();

}

interrupt_isr_uart()
{
   if (S0CON_SB_RI0) {
           S0CON_SB_RI0 = 0;
           /* don't overwrite chars already in buffer */
           if (rcnt < RBUFLEN)
                   rbuf [(unsigned char)(rpos+rcnt++) % RBUFLEN] = S0BUF;
   }
   if (S0CON_SB_TI0) {
           S0CON_SB_TI0 = 0;
		   busy = xcnt > 0;
           if (xcnt) {
                   xcnt--;
                   S0BUF = xbuf [xpos++];
                   if (xpos >= XBUFLEN)
                           xpos = 0;
           }
   }
}

void
putchar (char c)
{
   while (xcnt >= XBUFLEN) /* wait for room in buffer */
           ;
   interrupt_control_uart_disable();
   if (busy) {
           xbuf[(unsigned char)(xpos+xcnt++) % XBUFLEN] = c;
   } else {
           S0BUF = c;
           busy = 1;
   }
   interrupt_control_uart_enable();
}

char
getchar (void)
{
   unsigned char c;
   while (!rcnt)   /* wait for character */
           ;
   interrupt_control_uart_disable();
   rcnt--;
   c = rbuf [rpos++];
   if (rpos >= RBUFLEN)
           rpos = 0;
   interrupt_control_uart_enable();
   return (c);
}

void putstr(const char *s)
{
	for (; *s; s++)
		putchar(*s);
}

void putuint16(uint16_t val)
{
	char pbuf[8];
	uint8_t num_digits = 0;

	pbuf[0] = '0';
	for (; val > 0 && num_digits < sizeof(pbuf); val = val/10, num_digits++) {
		pbuf[num_digits] = '0' + (val % 10);
	}

	if (num_digits > 0)
		num_digits--;

	for (; num_digits > 0; num_digits--) {
		putchar(pbuf[num_digits]);
	}
	putchar(pbuf[0]);
}

void putfixed(uint16_t val)
{
	putuint16(val >> 2);
	putchar('.');
	putuint16( (val & 0x3) * 25 );
}

void putnibble(uint8_t val)
{
	val &= 0x0F;
	if (val < 10)
		putchar('0' + val);
	else
		putchar('A' + val - 10);
}

void puthex(uint8_t val)
{
	putnibble(val >> 4);
	putnibble(val);
}

unsigned char __xdata spritz_key[4*4];
uint8_t __xdata data_in[4*4];

void main()
{
	uint8_t i;
	uint8_t val;
	ser_init();

	putstr("Starting up\r\n");
	gpio_pin_configure(GPIO_PIN_ID_P1_6,
			GPIO_PIN_CONFIG_OPTION_DIR_OUTPUT |
			GPIO_PIN_CONFIG_OPTION_OUTPUT_VAL_CLEAR |
			GPIO_PIN_CONFIG_OPTION_PIN_MODE_OUTPUT_BUFFER_NORMAL_DRIVE_STRENGTH);
	interrupt_control_global_enable();

	delay_s(1);
	for (i = 0; i < sizeof(spritz_key)/4; i++) {
		spritz_key[i*4] = 'a';
		spritz_key[i*4+1] = 'b';
		spritz_key[i*4+2] = 'c';
		spritz_key[i*4+3] = 'd';
	}

	val = 'a';
	while(1)
	{
		for (i = 0; i < sizeof(data_in); i++)
			putchar(spritz_key[i]);
		putchar(' ');

		for (i = 0; i < sizeof(data_in); i++) {
			data_in[i] = val;
			putchar(data_in[i]);
		}
		putchar(' ');

		val++;
		if (val > 'z')
			val = 'a';

		gpio_pin_val_sbit_set(P1_SB_D6);
		spritz_encrypt(data_in, data_in, sizeof(data_in), NULL, 0, spritz_key, sizeof(spritz_key));
		gpio_pin_val_sbit_clear(P1_SB_D6);

		for (i = 0; i < sizeof(data_in); i++) {
			puthex(data_in[i]);
		}
		putstr("\r\n");
		delay_s(1);
	}
}
