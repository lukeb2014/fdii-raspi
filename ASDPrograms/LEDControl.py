import RPi.GPIO as gpio
from time import sleep 

redLED = 22
greenLED = 23
blueLED = 25


gpio.setwarnings(False)
gpio.setmode(gpio.BCM)

def turnLEDColor(color):
    if color == "red":
        gpio.cleanup()
        gpio.setwarnings(False)
        gpio.setmode(gpio.BCM)
        gpio.setup(redLED, gpio.OUT)
        gpio.output(redLED, gpio.LOW)
    elif color == "blue":
        gpio.cleanup()
        gpio.setwarnings(False)
        gpio.setmode(gpio.BCM)
        gpio.setup(blueLED, gpio.OUT)
        gpio.output(blueLED, gpio.LOW)
    elif color == "green":
        gpio.cleanup()
        gpio.setwarnings(False)
        gpio.setmode(gpio.BCM)
        gpio.setup(greenLED, gpio.OUT)
        gpio.output(greenLED, gpio.LOW)
    else:
        gpio.cleanup()
    
        