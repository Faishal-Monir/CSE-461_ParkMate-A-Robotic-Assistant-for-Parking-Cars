from gpiozero import LED, Button  
import time  

led = LED(18)  
button = Button(23)  

while True:  
    if button.is_pressed:  
        led.on()  
        print("Switch pressed") 
    else:  
        led.off() 
    print("Not pressed") 
    time.sleep(0.5)  
