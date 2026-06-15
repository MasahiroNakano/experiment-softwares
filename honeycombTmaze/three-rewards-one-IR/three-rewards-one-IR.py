"""three-rewards.py

Keyboard + logging front-end for three-reward Arduino controller.

Behavior:
    space : arm the next IR-triggered reward
    L     : overwrite next target to DIGITAL 8
    C     : overwrite next target to DIGITAL 9
    R     : overwrite next target to DIGITAL 10
    esc   : disarm, close serial port, and stop

The Arduino owns the IR detection, 50 ms pulse timing, suppression state,
and reward sequence. Python only sends commands and logs Arduino-reported events.
"""

import csv
import threading
import time
from datetime import datetime
from pathlib import Path

import serial
from pynput import keyboard


PORT = "/dev/cu.usbmodem1101"  # change to your actual Arduino port
BAUD = 115200


class ArduinoThreeRewardLogger:
    def __init__(self, port, baud=115200):
        self.ser = serial.Serial(port, baud, timeout=0.1)
        time.sleep(2)  # Arduino often resets when the serial port opens

        self.ser.reset_input_buffer()
        self.ser.reset_output_buffer()

        timestamp = datetime.now().strftime("%Y-%m-%d_%H%M%S")
        script_dir = Path(__file__).resolve().parent
        self.log_path = script_dir / f"three_reward_log_{timestamp}.csv"

        self.running = True
        self.closed = False
        self.serial_lock = threading.Lock()
        self.log_lock = threading.Lock()

        self.log_file = open(self.log_path, "w", newline="")
        self.writer = csv.writer(self.log_file)
        self.writer.writerow([
            "computer_time",
            "arduino_millis",
            "event",
            "pin",
            "raw_line",
        ])
        self.log_file.flush()

        self.reader_thread = threading.Thread(target=self.read_serial_loop, daemon=True)
        self.reader_thread.start()

        print(f"Connected to Arduino on {port}")
        print(f"Logging to {self.log_path}")
        print("Controls: space=arm, L=DIG8, C=DIG9, R=DIG10, esc=quit")

    def write_log_row(self, arduino_millis, event, pin, raw_line=""):
        computer_time = datetime.now().isoformat(timespec="milliseconds")

        with self.log_lock:
            self.writer.writerow([
                computer_time,
                arduino_millis,
                event,
                pin,
                raw_line,
            ])
            self.log_file.flush()

    def log_python_event(self, event, pin=-1):
        self.write_log_row("", event, pin, "")

    def read_serial_loop(self):
        while self.running:
            try:
                line = self.ser.readline().decode("utf-8", errors="replace").strip()

                if not line:
                    continue

                print(f"ARDUINO: {line}")

                # Expected Arduino format:
                # EVENT,<arduino_millis>,<event>,<pin>
                if line.startswith("EVENT,"):
                    parts = line.split(",", maxsplit=3)

                    if len(parts) == 4:
                        _, arduino_millis, event, pin = parts
                        self.write_log_row(arduino_millis, event, pin, line)
                    else:
                        self.write_log_row("", "UNPARSED_ARDUINO_LINE", -1, line)

            except serial.SerialException as exc:
                if self.running:
                    print(f"Serial read error: {exc}")
                break
            except Exception as exc:
                if self.running:
                    print(f"Unexpected read/log error: {exc}")

    def send_command(self, command, python_event=None, pin=-1):
        with self.serial_lock:
            self.ser.write(command.encode("ascii"))
            self.ser.flush()

        if python_event is not None:
            self.log_python_event(python_event, pin)

    def arm(self):
        self.send_command("a", "SPACE_ARM_COMMAND", -1)
        print("Reward armed: waiting for next IR beam break")

    def disarm(self):
        self.send_command("0", "DISARM_COMMAND", -1)
        print("Reward disarmed")

    def set_next_target(self, target):
        if target == 8:
            self.send_command("l", "KEY_SET_NEXT_TARGET", 8)
            print("Next target overwrite requested: DIG8")
        elif target == 9:
            self.send_command("c", "KEY_SET_NEXT_TARGET", 9)
            print("Next target overwrite requested: DIG9")
        elif target == 10:
            self.send_command("r", "KEY_SET_NEXT_TARGET", 10)
            print("Next target overwrite requested: DIG10")
        else:
            raise ValueError("target must be 8, 9, or 10")

    def close(self):
        if self.closed:
            return

        self.closed = True

        try:
            self.disarm()
            time.sleep(0.2)  # allow Arduino DISARMED event to arrive
        except Exception as exc:
            print(f"Error while disarming during close: {exc}")

        self.running = False
        self.reader_thread.join(timeout=1)

        try:
            self.ser.close()
        finally:
            self.log_file.close()

        print("Closed serial connection and log file")


reward = ArduinoThreeRewardLogger(PORT, BAUD)
space_down = False


def on_press(key):
    global space_down

    if key == keyboard.Key.space and not space_down:
        space_down = True
        reward.arm()
        return

    if key == keyboard.Key.esc:
        reward.close()
        return False

    try:
        char = key.char.lower()
    except AttributeError:
        return

    if char == "l":
        reward.set_next_target(8)
    elif char == "c":
        reward.set_next_target(9)
    elif char == "r":
        reward.set_next_target(10)


def on_release(key):
    global space_down

    if key == keyboard.Key.space:
        space_down = False


try:
    with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
        listener.join()
except KeyboardInterrupt:
    pass
finally:
    reward.close()
