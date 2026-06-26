import RPi.GPIO as GPIO
from gpiozero import LED, Button
import time

GPIO.setmode(GPIO.BCM)
TRIG = 21  # GPIO21
ECHO = 20  # GPIO20
led = LED(18) 

GPIO.setup(TRIG, GPIO.OUT)
GPIO.setup(ECHO, GPIO.IN)

def distance():
    GPIO.output(TRIG, False)
    time.sleep(0.5)
    GPIO.output(TRIG, True)
    time.sleep(0.00001)
    GPIO.output(TRIG, False)
    pulse_start = time.time()
    while GPIO.input(ECHO) == 0:
        pulse_start = time.time()
    while GPIO.input(ECHO) == 1:
        pulse_end = time.time()
    pulse_duration = pulse_end - pulse_start
    distance_val = pulse_duration * ((343 * 100) / 2)  
    distance_val = round(distance_val, 2)
    return distance_val

while True:
    dis = distance()
    print(dis)
    if dis < 5:
        led.on()
    else:
        led.off()

GPIO.cleanup()
