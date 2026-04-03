#!/usr/bin/env python3
import time, signal, os

signal.signal(signal.SIGINT,  lambda *_: os._exit(0))
signal.signal(signal.SIGTERM, lambda *_: os._exit(0))

# Send valid CGI headers first (optional)
print("Content-Type: text/plain\r\n\r")

# Now hang forever
while True:
    time.sleep(1)