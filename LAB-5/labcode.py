import serial
from rpi_lcd import LCD
from time import sleep
from gpiozero import LED, Button  
import time  



# Initialize LCD
lcd = LCD()

#led setup
led1 = LED(18) 
led2 = LED(19) 
led3 = LED(20) 


# Set up serial connection
ser = serial.Serial('/dev/ttyS0', 9600, timeout=1)

try:
    while True:
        if ser.in_waiting > 0:
            # Read and decode the line from Arduino
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            
            # Clear LCD and display the received message
            lcd.clear()
            lcd.text("IR Status: ", 1)
            lcd.text(line, 2)
            
            if line=="LEFT":
                led1.on()
                led2.off()
                led3.off()
            elif line=="RIGHT":
                led1.off()
                led2.off()
                led3.on()
            elif line=="FORWARD":
                led1.on()
                led2.on()
                led3.on()
            else:
                led1.off()
                led2.off()
                led3.off()
            
            
            
except KeyboardInterrupt:
    lcd.clear()
    ser.close()
    print("Program terminated")

