import time
import serial
from pynput import keyboard


class ArduinoThreeReward:
    def __init__(self, port, baud=115200):
        self.ser = serial.Serial(port, baud)
        time.sleep(2)

        self.ser.reset_input_buffer()
        self.ser.reset_output_buffer()

        # Repeating sequence:
        # 8 → 9 → 10 → 9 → 8 → ...
        self.sequence = [8, 9, 10, 9]
        self.sequence_index = 0

    def pulse_dig8(self):
        self.ser.write(b"8")
        self.ser.flush()

    def pulse_dig9(self):
        self.ser.write(b"9")
        self.ser.flush()

    def pulse_dig10(self):
        # Arduino receives 't' as command for DIGITAL 10
        self.ser.write(b"t")
        self.ser.flush()

    def pulse_target(self, target):
        if target == 8:
            print("Pulsing DIG8")
            self.pulse_dig8()

        elif target == 9:
            print("Pulsing DIG9")
            self.pulse_dig9()

        elif target == 10:
            print("Pulsing DIG10")
            self.pulse_dig10()

        else:
            raise ValueError("target must be 8, 9, or 10")

    def pulse_next_target(self):
        target = self.sequence[self.sequence_index]

        self.pulse_target(target)

        # Advance to next position in 8 → 9 → 10 → 9 sequence
        self.sequence_index = (self.sequence_index + 1) % len(self.sequence)

        next_target = self.sequence[self.sequence_index]
        print(f"Next target: DIG{next_target}")

    def set_next_target(self, target):
        """
        Overwrite the next target.
        For DIG9, this chooses the first DIG9 position in the sequence,
        so after DIG9 the next target will be DIG10.
        """
        if target == 8:
            self.sequence_index = 0
        elif target == 9:
            self.sequence_index = 1
        elif target == 10:
            self.sequence_index = 2
        else:
            raise ValueError("target must be 8, 9, or 10")

        print(f"Next target overwritten: DIG{target}")

    def stop(self):
        self.ser.write(b"0")
        self.ser.flush()

    def close(self):
        self.stop()
        self.ser.close()


PORT = "/dev/cu.usbmodem1101"  # change to your actual Arduino port

reward = ArduinoThreeReward(PORT)

space_down = False


def on_press(key):
    global space_down

    # Space: pulse current target, then advance sequence
    if key == keyboard.Key.space and not space_down:
        space_down = True
        reward.pulse_next_target()
        return

    # Optional manual overwrite keys
    try:
        char = key.char.lower()

        if char == "l":
            reward.set_next_target(8)    # L → next target DIG8

        elif char == "c":
            reward.set_next_target(9)    # C → next target DIG9

        elif char == "r":
            reward.set_next_target(10)   # R → next target DIG10

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