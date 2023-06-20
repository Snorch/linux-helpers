#!/bin/bash
PATTERN=$1
for nspid in $(lsns -t mnt | tail -n +2 | awk '{print $4}'); do
	grep "$PATTERN" /proc/$nspid/mountinfo &&
		echo "Nspid $nspid"
done
