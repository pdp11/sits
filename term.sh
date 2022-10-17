#!/bin/sh

PORT="$1"
NAME="$2"

printf "\033]0;$NAME\007"

while :; do
    telnet localhost "$PORT"
    sleep 0.1
done
