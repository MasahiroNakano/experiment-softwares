# https://chatgpt.com/g/g-p-6a2fba68cd2c8191a54b5a1525b35f14-experimental-neurosciecne/c/6a2fba70-44e0-83e8-8fb7-c89e11c00e3a

import serial.tools.list_ports

for port in serial.tools.list_ports.comports():
    print(port.device, "-", port.description)