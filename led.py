""" This program is used to control the RGB LED strips connected to the Raspberry Pi. 
    It turns on/off 3 GPIO pins that control 3 transistors. 
    These transistors control the 3 individual colors.
    """
import RPi.GPIO as GPIO
import time
import sys

""" Define the custom functions that controls the output pins """
def all_off():
    GPIO.output(blue_pin,GPIO.LOW)
    GPIO.output(red_pin,GPIO.LOW)
    GPIO.output(green_pin,GPIO.LOW)

def red():
    GPIO.output(red_pin,GPIO.HIGH)

def blue():
    GPIO.output(blue_pin,GPIO.HIGH)

def green():
    GPIO.output(green_pin,GPIO.HIGH)

def yellow():
    red()
    green()

def cyan():
    blue()
    green()

def pink():
    blue()
    red()

def white():
   # blue()
    #red()
    #green()
    all_off()

def cw():
    try:
        print "Executing"
        print "Press 'Ctrl-C' to go back and choose another color"
        while 1:
            """Blue"""
            blue()
            time.sleep(.5)
            
            """Pink"""
            red()
            time.sleep(.5)
            
            """Red"""
            all_off()
            red()
            time.sleep(.5)
            
            """Yellow"""
            green()
            time.sleep(.5)
            
            """Green"""
            all_off()
            green()
            time.sleep(.5)
            
            """Cyan"""
            blue()
            time.sleep(.5)
                
            all_off()
            
    except KeyboardInterrupt:
        pass
            
def wvu():
    try:
        print "Executing"
        print "Press 'Ctrl-C' to go back and choose another color"
        all_off()
        while 1:
            """ Blue """
            blue()
            time.sleep(1)
            
            """ Yellow """
            all_off()
            yellow()
            time.sleep(1)
            
            all_off()
            
    except KeyboardInterrupt:
        pass

def print_options():
    print """-------------Solar Tree LED Controller-------------

Choose an option
1. Red
2. Blue
3. Green
4. Pink
5. Yellow
6. Cyan
7. White
8. Color Wheel
9. WVU

"""
            
""" ----------------------------------------------------------------------------------------------------------------------------- """

""" Array of color options the user can call """
options = {1 : red,
           2 : blue,
           3 : green,
           4 : pink,
           5 : yellow,
           6 : cyan,
           7 : white,
           8 : cw,
           9 : wvu,
}

""" Set the pin variables to which GPIO pins are used to control each color """
red_pin = 12
blue_pin  = 16
green_pin = 26

""" Variable to control the input loop """
loop_stop = 0

""" Set up the GPIO pins """
GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)

""" Initialize the 3 LEDs as OUTPUT """
GPIO.setup(green_pin,GPIO.OUT)
GPIO.setup(red_pin,GPIO.OUT)
GPIO.setup(blue_pin,GPIO.OUT)

all_off()

""" If no command line arguments are passed, loop and ask what colors to use """ 
if len(sys.argv) == 1:
    while loop_stop==0:
        print_options()
        try:    
            print "Press 'Ctrl-C' to exit back to terminal"            
            """ Take in the users choice of color """
            choice = input('Your choice: ')
            """ Execute any of the regular color options """
            if choice > 0 and choice <= len(options):
                all_off()
                options[choice]()
        except KeyboardInterrupt:
            loop_stop=1
            pass
        """ If there was a command line argument passed, parse it and compare to the usable colors """
elif len(sys.argv) == 2:
    if 'red' == str(sys.argv[1]):
        all_off()
        red()
    elif 'blue' == str(sys.argv[1]):
        all_off()
        blue()
    elif 'green' == str(sys.argv[1]):
        all_off()
        green()
    elif 'yellow' == str(sys.argv[1]):
        all_off()
        yellow()
    elif 'cyan' == str(sys.argv[1]):
        all_off()
        cyan()
    elif 'pink' == str(sys.argv[1]):
        all_off()
        pink()
    elif 'white' == str(sys.argv[1]):
        all_off()
        white()
    elif 'wvu' == str(sys.argv[1]):
        all_off()
        wvu()
    elif 'cw' == str(sys.argv[1]):
        all_off()
        cw()

print "\n\nExiting Program\n\n"
