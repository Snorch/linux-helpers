#!/bin/bash

if [ "$#" -gt 2 ] || [ "$#" -lt 1 ]; then
	echo "usage: $0 <command> [<path/to/executable>]"
	exit 1
fi

COMM=$1
PIDS=$(pgrep $COMM)
COMM_PATH=$(which $COMM)
if [ "$#" -eq 2 ]; then
	COMM_PATH=$2
fi

for i in $PIDS; do
	cat /proc/$i/stack | grep sleep &>/dev/null && PID=$i && break
done

if [ -z ${PID+x} ]; then
	echo "Can't find sleeping criu"
	exit 1
fi

echo Attaching $PID with gdb
gdb -p $PID $COMM_PATH
