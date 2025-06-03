@echo off

IF NOT EXIST ".\build" mkdir ".\build"

pushd ".\build"
sdcc -mmcs51 --iram-size 0x100 --xram-size 0x00 ../src/main.c
packihx main.ihx > main.hex
popd
