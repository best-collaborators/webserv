#!/usr/bin/env python3
import time

# Send valid CGI headers first (optional)
print("Content-Type: text/plain\r\n\r")

# Now hang forever
while True:
    time.sleep(1)