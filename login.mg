#!/bin/bash
/bin/fflush
/usr/bin/stty -echo

sleep 1.5
echo -ne +++
sleep 1.5
echo ATS2=128
read -t 0.1 -N 128 discard
read -t 0.5 -N 128 discard

echo ATO0
read -t 0.1 -N 128 discard
read -t 0.5 -N 128 discard

read -t 0.2 -N 128 discard
read -t 1.5 -N 128 discard


/usr/bin/stty echo
echo -ne '\b\b\b'

exec /bin/login $@
