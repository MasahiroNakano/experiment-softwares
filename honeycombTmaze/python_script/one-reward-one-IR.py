import time
import serial
from pynput import keyboard


class ArduinoIRReward:
    def __init__(self, port, baud=115200):
        self.ser = serial.Serial(port, baud)
        time.sleep(2)

        self.ser.reset_input_buffer()
        self.ser.reset_output_buffer()

    def arm(self):
        self.ser.write(b"a")
        self.ser.flush()
        print("Reward armed")

    def disarm(self):
        self.ser.write(b"0")
        self.ser.flush()
        print("Reward disarmed")

    def close(self):
        self.disarm()
        self.ser.close()


PORT = "/dev/cu.usbmodem1101"  # change to your actual Arduino port

reward = ArduinoIRReward(PORT)

space_down = False


def on_press(key):
    global space_down

    if key == keyboard.Key.space and not space_down:
        space_down = True
        reward.arm()

    if key == keyboard.Key.esc:
        reward.close()
        return False


def on_release(key):
    global space_down

    if key == keyboard.Key.space:
        space_down = False


with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
    listener.join()