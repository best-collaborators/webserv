#!/usr/bin/env python3
import time
import sys

print("Content-Type: text/plain\r\n\r")
sys.stdout.flush()

# Delay body
time.sleep(5)

print("This should never reach client if timeout is 5s")