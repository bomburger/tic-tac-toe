import serial
import time

# COM1 - proteus
# COM2 - qt
ser = serial.Serial('COM2', 9600)  # use the paired port

ser.write(b'b')  # send one byte
print("Sent 'b'")

ser.close()

