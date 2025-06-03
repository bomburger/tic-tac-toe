import serial
import time

ser = serial.Serial('COM1', 9600)  # use the paired port

ser.write(b'b')  # send one byte
print("Sent 'b'")

ser.close()

