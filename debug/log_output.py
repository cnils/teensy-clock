import re
import time
import serial

# connect to serial port
ser = serial.Serial('COM3', 57600)
ser.flushInput()

# log data
while True:
    decoded_bytes = ser.readline().decode('utf-8').strip()
    board_time = re.search('\s(\d+\:.*)$', decoded_bytes).group(1)
    struct_time = time.strptime(board_time, '%H:%M:%S')
    formatted_time = time.strftime('%H:%M:%S', struct_time)
    with open('drift_log.txt','a+') as f:
        f.write(f'{time.strftime("%H:%M:%S")}, {formatted_time}\n')