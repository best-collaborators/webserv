#!/bin/bash

while true; do
    ZOMBIES=$(ps aux | awk '$8 ~ /^Z/ {print $2}')

    if [ -n "$ZOMBIES" ]; then
        echo "Zombie processes detected: $ZOMBIES"
    fi

    sleep 1
done