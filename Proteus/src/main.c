#include <8051.h>
#include <stdint.h>

// bruh
void delay_t(uint32_t ms){
	for (uint32_t i = 0; i < ms; i++){}
}

// initializing com port
void UART_init(void){ 
	TMOD = 0x20;
	SCON = 0x50;
	TH1 = 0xFD;
	TL1 = 0xFD;
	TR1 = 1;
}

// sending 1 byte
void putc(uint8_t input){
	SBUF = input;
	while (TI != 1){}
	TI = 0;
}

//показывает, если ли в буфере непрочитанное значение
int UART_is_not_Empty(void){
	return RI;
}

//возвращает значение из буфера
char UART_read(void){
	RI = 0;
	return SBUF;
}

void main(void) {
	UART_init();	
	while(1) {
		if (UART_is_not_Empty()) {
			char input = UART_read();
			putc('c');
		}
	}
}
