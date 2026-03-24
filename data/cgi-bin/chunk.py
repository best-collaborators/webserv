#!/usr/bin/env python3
import time
import sys

print("Content-Type: text/plain\r\n\r")
sys.stdout.flush()

for i in range(5):
    print(f"Chunk {i}")
    sys.stdout.flush()
    time.sleep(1)