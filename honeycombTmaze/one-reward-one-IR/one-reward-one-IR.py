import csv
import time
import threading
from datetime import datetime

import serial
from pynput import keyboard


class ArduinoIRRewardLogger:
    def __init__(self, port, baud=115200):
        self.ser = serial.Serial(port, baud, timeout=0.1)
        time.sleep(2)

        self.ser.reset_input_buffer()
        self.ser.reset_output_buffer()

        timestamp = datetime.now().strftime("%Y-%m-%d_%H%M%S")
        self.log_filename = f"reward_log_{timestamp}.csv"

        self.lock = threading.Lock()
        self.running = True

        self.log_file = open(self.log_filename, "w", newline="")
        self.writer = csv.writer(self.log_file)

        self.writer.writerow([
            "computer_time",
            "arduino_millis",
            "event",
            "pin"
        ])
        self.log_file.flush()

        self.reader_thread = threading.Thread(target=self.read_serial_loop)
        self.reader_thread.start()

        print(f"Logging to {self.log_filename}")

    def log_python_event(self, event, pin=-1):
        computer_time = datetime.now().isoformat(timespec="milliseconds")

        self.writer.writerow([
            computer_time,
            "",
            event,
            pin
        ])
        self.log_file.flush()

    def read_serial_loop(self):
        while self.running:
            try:
                line = self.ser.readline().decode("utf-8", errors="replace").strip()

                if not line:
                    continue

                print(line)

                # Expected Arduino format:
                # EVENT,12345,IR_TRIGGER,2
                if line.startswith("EVENT,"):
                    parts = line.split(",")

                    if len(parts) == 4:
                        _, arduino_millis, event, pin = parts

                        computer_time = datetime.now().isoformat(timespec="milliseconds")

                        self.writer.writerow([
                            computer_time,
                            arduino_millis,
                            event,
                            pin
                        ])
                        self.log_file.flush()

            except Exception as e:
                print("Serial read error:", e)

    def arm(self):
        with self.lock:
            self.ser.write(b"a")
            self.ser.flush()

        self.log_python_event("SPACE_ARM_COMMAND")
        print("Reward armed")

    def disarm(self):
        with self.lock:
            self.ser.write(b"0")
            self.ser.flush()

        self.log_python_event("DISARM_COMMAND")
        print("Reward disarmed")

    def close(self):
        self.running = False
        time.sleep(0.2)

        try:
            self.disarm()
        except Exception:
            pass

        self.reader_thread.join(timeout=1)

        self.ser.close()
        self.log_file.close()

        print("Closed logger")


PORT = "/dev/cu.usbmodem1101"  # change this to your actual Arduino port

reward = ArduinoIRRewardLogger(PORT)

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