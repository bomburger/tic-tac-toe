import serial
import time

ser = serial.Serial('COM2', 9600)  # use the paired port

ser.write(b'b')  # send one byte
print("Sent 'b'")

data = ser.read()  # read echo from MCU
print("Received:", data)

ser.close()

