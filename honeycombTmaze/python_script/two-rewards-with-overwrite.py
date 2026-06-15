import time
import serial
from pynput import keyboard


class ArduinoTwoReward:
    def __init__(self, port, baud=115200):
        self.ser = serial.Serial(port, baud)
        time.sleep(2)

        self.ser.reset_input_buffer()
        self.ser.reset_output_buffer()

        self.next_target = "8"  # first space press pulses DIG8

    def pulse_dig8(self):
        self.ser.write(b"8")
        self.ser.flush()

    def pulse_dig9(self):
        self.ser.write(b"9")
        self.ser.flush()

    def set_next_target(self, target):
        if target not in ("8", "9"):
            raise ValueError("target must be '8' or '9'")

        self.next_target = target
        print(f"Next target overwritten: DIG{target}")

    def pulse_next_target(self):
        if self.next_target == "8":
            print("Pulsing DIG8")
            self.pulse_dig8()
            self.next_target = "9"
            print("Next target: DIG9")

        elif self.next_target == "9":
            print("Pulsing DIG9")
            self.pulse_dig9()
            self.next_target = "8"
            print("Next target: DIG8")

    def stop(self):
        self.ser.write(b"0")
        self.ser.flush()

    def close(self):
        self.stop()
        self.ser.close()


PORT = "/dev/cu.usbmodem1101"  # change this to your actual port

reward = ArduinoTwoReward(PORT)

space_down = False


def on_press(key):
    global space_down

    # Space: pulse current target, then toggle
    if key == keyboard.Key.space and not space_down:
        space_down = True
        reward.pulse_next_target()
        return

    # Letter keys: overwrite next target
    try:
        char = key.char.lower()

        if char == "l":
            reward.set_next_target("8")  # L → DIG8

        elif char == "c":
            reward.set_next_target("9")  # C → DIG9

    except AttributeError:
        pass


def on_release(key):
    global space_down

    if key == keyboard.Key.space:
        space_down = False

    if key == keyboard.Key.esc:
        reward.close()
        return False


with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
    listener.join()