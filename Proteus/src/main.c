#include <8051.h>
#include <stdint.h>

#define uint8 uint8_t
#define blink_delay 10000

#define R1_PIN P1_0
#define R2_PIN P1_1
#define R3_PIN P1_2
#define C1_PIN P1_3
#define C2_PIN P1_4
#define C3_PIN P1_5
#define X_PIN  P1_6
#define O_PIN  P1_7

#define CURSOR_PIN P0_0
#define SELECT_PIN P0_1

uint8 board[9]; // 0 - nothing; 1 - X; 2 - O;
uint8 cursor;
uint8 player;

// initializing com port
void UART_init(void){ 
	TMOD = 0x20;
	SCON = 0x50;
	TH1 = 0xFD;
	TL1 = 0xFD;
	TR1 = 1;
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

// send 1 byte
void UART_write(uint8 data){
	SBUF = data;
	while (TI != 1);
	TI = 0;
}

void reset_board(void) {
	for (int i = 0; i < 9; i++) {
		board[i] = 0;
	}
}

void reset_board_visual(void) {
	P1 = 0b00111000; // 0,1,2 - rows; 3,4,5 - columns; 6,7 - X and O;
}

// we can display the board with just 8 pins.
// 3 pins - rows
// 3 pins - columns
// 2 pins - X or O
// need 2 transistors for each row

void scan_display(void) {
	for (uint8 r = 0; r < 3; r++) {
		reset_board_visual();
        
        // Activate only row “r”
        if (r == 0) {
		   	R1_PIN = 1;
		}
        else if (r == 1) {
		   	R2_PIN = 1; 
		}
        else {
		   	R3_PIN = 1; 
		}
        
        // to light led we do (row=1, col=0).

		// drawing X's
		X_PIN = 1;
		O_PIN = 0;
        for (uint8 c = 0; c < 3; c++) {
            uint8 idx = r*3 + c;
            if (board[idx] == 1) {
				if      (c == 0) C1_PIN = 0;
				else if (c == 1) C2_PIN = 0;
				else if (c == 2) C3_PIN = 0;
			}
        }
		// delay
        for (uint16_t d = 0; d < blink_delay; d++);
		// drawing O's
		X_PIN = 0;
		O_PIN = 1;
		C1_PIN = 1;
		C2_PIN = 1;
		C3_PIN = 1;
        for (uint8 c = 0; c < 3; c++) {
            uint8 idx = r*3 + c;
            if (board[idx] == 2) {
				if      (c == 0) C1_PIN = 0;
				else if (c == 1) C2_PIN = 0;
				else if (c == 2) C3_PIN = 0;
			}
        }
		// delay
        for (uint16_t d = 0; d < blink_delay; d++);
    }
}

void process_data(void) {
	if (UART_is_not_empty()) {
		uint8 data = UART_read();
		// debug
		UART_write(data);

		// FORMAT:
		// r00scccc
		// r - reset the board
		// s - shape: 0 - X, 1 - O
		// c - cell (standart 8421)
		if (data & (1 << 7)) {
			reset_board();
			reset_board_visual();
			return;
		}
		int shape = (data & (1 << 4)) == 0? 1 : 2; // maps 0,1 -> 1,2
		int cell = data & 0b00001111; // just take 4 least bits
		board[cell] = shape;
	}

	// FORMAT:
	// ccccdddd
	// c - code
	// d - data
	// for cursor:
	// cccc <- 1000; dddd <- cursor
	// for select:
	// cccc <- 0100; dddd <- 0000
	if (CURSOR_PIN == 0) {
		uint8 data = 0;
		data |= (1 << 7);
		cursor = (cursor + 1) % 9;
		data |= cursor;
		UART_write(data);
	} else if (SELECT_PIN == 0) {
		uint8 data = 0 | (1 << 6);
		UART_write(data);
	}
}

void main(void) {
	UART_init();
	reset_board();
	reset_board_visual();
	while(1) {
		process_data();
		scan_display();
	}
}
