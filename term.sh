#!/bin/sh

PORT="$1"

while :; do
    telnet localhost "$PORT"
    sleep 0.1
done
