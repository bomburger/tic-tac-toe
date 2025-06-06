# 🌐 Available Languages

- [English](README.md)
- [Русский](README_ru.md)

## Tic Tac Toe

Tic Tac Toe game using an 80C31 microcontroller simulated in Proteus, with a Qt app communicating via COM port.

---

## How to run it

1. Clone this repo.

    ```shell
    git clone https://github.com/bomburger/tic-tac-toe
    cd tic-tac-toe
    ```

2. Open proteus project: `Proteus/tictac.pdsprj`
3. Simulate COM ports COM1 <=> COM2. [VSPE](https://eterlogic.com/Products.VSPE.html) can do this.
4. Build & run Qt app (with qmake). Project is located in `Qt/Project.pro`
5. Run proteus simulation.

---

## How it works

There are 2 programs:

- Running on microcontroller simulated in Proteus.
- Qt app.

Each program is capable of displaying and changing state of the game.
Programs communicating via COM port.

### Proteus simulation

#### Hardware

1. 80C31 chip.
2. 18 LEDs (+ 6 BJT NPN transistors) to display 3x3 field.
3. 2 buttons for selecting & placing shapes.
4. 7 segment display to show which cell is selected.
5. LED for displaying who's turn is it.

#### Program

Code for the program running on MCU can be found at `Proteus/src/main.c`
To compile it, use Proteus/build.bat (requires [sdcc](https://sdcc.sourceforge.net/) to be in PATH).

```shell
cd Proteus
.\build.bat
```

This outputs main.hex file in `Proteus/build` directory. This file is set to be a program file on the MCU.

### Qt app

Qt project is located at `Qt/Project.pro`
You can build it with Qt Creator or manually with qmake and make.

### COM port

For programs to communicate, create virtual pair COM1 <=> COM2. You can use [VSPE](https://eterlogic.com/Products.VSPE.html) for this.
COM port has following parameters:

- Baudrate: 9600
- Data bits: 8
- Parity: none
- Stop bits: 1

### Communication Format

#### Qt -> MCU

To send messages from Qt app to MCU program, fo;lowing format is used:

1 byte message: **rc0sdddd**

- r - this bit is set if the game ended & everything should be reset.
- c - this bit is set, when in Qt app the user selected a cell. If this bit is 1, data bits(d) show which cell is selected.
- if both r and c are 0, a shape was placed on the board. s indicates which shape (0 - X, 1 - O), data bits(d) indicate the cell.

#### MCU -> Qt

To send messages from MCU to Qt app, following format is used.

1 byte message: **cs00dddd**

- c - this bit is set when cursor shift button is pressed. Increments cursor by 1. When this bit is set, data bits(d) show new value of cursor.
- s - this bit is set when select button was pressed. This means, place shape on currently selected cell. In this case, data bits(d) are set to 0.
