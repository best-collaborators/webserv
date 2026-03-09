#!/usr/bin/env python3
import time
import sys

print("Content-Type: text/plain")
print()
sys.stdout.flush()

for i in range(20):
    print(f"Chunk {i}")
    sys.stdout.flush()
    time.sleep(1)