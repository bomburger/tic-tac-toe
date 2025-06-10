#include <8051.h>
#include <stdint.h>

#define uint8 uint8_t

#define X1 P1_0
#define X2 P1_1
#define X3 P1_2
#define X4 P1_3
#define X5 P1_4
#define X6 P1_5
#define X7 P1_6
#define X8 P1_7
#define X9 P0_2
#define O1 P0_3
#define O2 P0_4
#define O3 P0_5
#define O4 P0_6
#define O5 P0_7
#define O6 P2_5
#define O7 P2_6
#define O8 P2_7
#define O9 P0_1

#define A P2_0
#define B P2_1
#define C P2_2
#define D P2_3

#define TURN_PIN P2_4 // should go low to light up led

static uint8 cursor; // 1 - X, 2 - O;
static uint8 board[9];
static uint8 byte_to_send;
static uint8 is_byte_to_send;

// initializing com port
void UART_init(void) { 
	TMOD = 0x20;
	SCON = 0x50;
	TH1 = 0xFD;
	TL1 = 0xFD;
	TR1 = 1;
}

void configure_interrupts(void) {
	IT0 = 1;
	IT1 = 1;
	EX0 = 1;
	EX1 = 1;
	ES = 1;
	EA = 1;
}

// shows if there is data in com port
int UART_is_not_empty(void){
	return RI;
}

// read 1 byte
uint8 UART_read(void){
	RI = 0;
	return SBUF;
}

void add_byte_to_write_queue(uint8 byte) {
	if (is_byte_to_send) { return; }
	byte_to_send = byte;
	is_byte_to_send = 1;
}

void reset_led_matrix(void) {
	X1 = 1;
	X2 = 1;
	X3 = 1;
	X4 = 1;
	X5 = 1;
	X6 = 1;
	X7 = 1;
	X8 = 1;
	X9 = 1;
	O1 = 1;
	O2 = 1;
	O3 = 1;
	O4 = 1;
	O5 = 1;
	O6 = 1;
	O7 = 1;
	O8 = 1;
	O9 = 1;
}

void reset_num_display(void) {
	A = 0;
	B = 0;
	C = 0;
	D = 0;
}

void display_digit(int digit) {
	A = digit & (1 << 0) ? 1 : 0;
	B = digit & (1 << 1) ? 1 : 0;
	C = digit & (1 << 2) ? 1 : 0;
	D = digit & (1 << 3) ? 1 : 0;
}

void display_shape(int cell, int shape) {
	if (shape == 0) {
		// None
		switch(cell) {
			case 0:
				X1 = 1;
				O1 = 1;
				break;
			case 1:
				X2 = 1;
				O2 = 1;
				break;
			case 2:
				X3 = 1;
				O3 = 1;
				break;
			case 3:
				X4 = 1;
				O4 = 1;
				break;
			case 4:
				X5 = 1;
				O5 = 1;
				break;
			case 5:
				X6 = 1;
				O6 = 1;
				break;
			case 6:
				X7 = 1;
				O7 = 1;
				break;
			case 7:
				X8 = 1;
				O8 = 1;
				break;
			case 8:
				X9 = 1;
				O9 = 1;
				break;
		}
	}
	if (shape == 1) { 
		// X
		switch(cell) {
			case 0:
				X1 = 0;
				break;
			case 1:
				X2 = 0;
				break;
			case 2:
				X3 = 0;
				break;
			case 3:
				X4 = 0;
				break;
			case 4:
				X5 = 0;
				break;
			case 5:
				X6 = 0;
				break;
			case 6:
				X7 = 0;
				break;
			case 7:
				X8 = 0;
				break;
			case 8:
				X9 = 0;
				break;
		}
	}
	if (shape == 2) { 
		// O
		switch(cell) {
			case 0:
				O1 = 0;
				break;
			case 1:
				O2 = 0;
				break;
			case 2:
				O3 = 0;
				break;
			case 3:
				O4 = 0;
				break;
			case 4:
				O5 = 0;
				break;
			case 5:
				O6 = 0;
				break;
			case 6:
				O7 = 0;
				break;
			case 7:
				O8 = 0;
				break;
			case 8:
				O9 = 0;
				break;
		}
	}
}

void reset(void) {
	for (int i = 0; i < 9; i++) { board[i] = 0; }
	cursor = 0;
	reset_led_matrix();
	reset_num_display();
	display_digit(1);
	display_shape(0, 1);
	display_shape(0, 2);
	TURN_PIN = 1;
}

void serial_interrupt(void) __interrupt(4) {
	// FORMAT:
	// rcxsdddd
	// r - reset the board
	// c - cursor
	// s - placed shape: 0 - X, 1 - O
	// d - if r: 0; if c or s: cell
	if (RI) {
		RI = 0;
		uint8 data = SBUF;

		if (data & (1 << 7)) {
			reset();
		} else if (data & (1 << 6)) {
			// cursor placement
			display_shape(cursor, 0);
			display_shape(cursor, board[cursor]);
			cursor = data & 0x0F;
			display_shape(cursor, 1);
			display_shape(cursor, 2);
			display_digit(cursor + 1);
		} else {
			// shape placement
			int shape = (data & (1 << 4)) == 0? 1 : 2; // maps 0,1 -> 1,2
			int cell = data & 0x0F; // just take 4 least bits
				
			display_shape(cell, shape);

			TURN_PIN = shape - 1;
			board[cell] = shape;
		}
	}
}

// ccccdddd
// for cursor:
// cccc <- 1000; dddd <- cursor
void int0_interrupt(void) __interrupt(0) {
	// Move cursor
	display_shape(cursor, 0);
	display_shape(cursor, board[cursor]);
	cursor = (cursor + 1) % 9;
	display_shape(cursor, 1);
	display_shape(cursor, 2);
	display_digit(cursor + 1);
	uint8 data = 0;
	data |= (1 << 7);
	data |= cursor;
	add_byte_to_write_queue(data);
}

// ccccdddd
// for select:
// cccc <- 0100; dddd <- 0000
void int1_interrupt(void) __interrupt(2) {
	// Place shape
	uint8 data = 0 | (1 << 6);
	add_byte_to_write_queue(data);
}

void main(void) {
	UART_init();
	reset();
	configure_interrupts();
	while(1) {
		if (is_byte_to_send == 1) {
			SBUF = byte_to_send;
			TI = 1;
			while (TI != 1);
			TI = 0;
			is_byte_to_send = 0;
		}
	}
}

