import time
import serial
from pynput import keyboard

class ArduinoSolenoid:
    def __init__(self, port, baud=115200):
        self.ser = serial.Serial(port, baud)
        time.sleep(2)  # Arduino often resets when serial opens

    def pulse(self):
        self.ser.write(b"p")

    def stop(self):
        self.ser.write(b"0")

    def close(self):
        self.stop()
        self.ser.close()


# solenoid = ArduinoSolenoid("COM3")  # change this port
PORT = "/dev/cu.usbmodem1101"
solenoid = ArduinoSolenoid(PORT)  

space_down = False

def on_press(key):
    global space_down

    if key == keyboard.Key.space and not space_down:
        space_down = True
        solenoid.pulse()
        print("Pulse")

def on_release(key):
    global space_down

    if key == keyboard.Key.space:
        space_down = False

    if key == keyboard.Key.esc:
        solenoid.close()
        return False


with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
    listener.join()