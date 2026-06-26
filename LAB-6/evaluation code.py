import RPi.GPIO as GPIO
import time
from gpiozero.pins.pigpio import PiGPIOFactory
from gpiozero import Device, Servo, AngularServo
from time import sleep

GPIO.setmode(GPIO.BCM)
TRIG = 21
ECHO = 20

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
    distance = pulse_duration * ((343 * 100) / 2)
    distance = round(distance, 2)

    return distance

Device.pin_factory = PiGPIOFactory()

s = AngularServo(17, min_angle=0, max_angle=180,min_pulse_width=0.5/1000, max_pulse_width=25/10000)

while True:
    lenght = distance()
    print("Distance:", lenght, "cm")

    if lenght < 5:
        s.angle = 180
        print("\nGate Has been Opened:\n")
        sleep(0.1)
    else:
        s.angle = 90
        print("\nGate Has been Closed:\n")
        sleep(0.1)

GPIO.cleanup()





