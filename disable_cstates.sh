#!/bin/bash

if [ $# -ge 1 ] && [ "$1" = "reenable" ]; then
	echo 0 | tee /sys/devices/system/cpu/cpu*/cpuidle/state*/disable
	exit 0
fi

# Get max C-state
MAX_STATE=$(ls /sys/devices/system/cpu/cpu0/cpuidle/state* -d | tail -n 1 | sed 's/.*state//')

# Disable CPU C-states except 0-th
for STATE in $(seq 1 $MAX_STATE); do
	# Disable the C-state $STATE on all CPUs
	echo 1 | tee /sys/devices/system/cpu/cpu*/cpuidle/state$STATE/disable
done

echo "To re-enable C-states: $0 reenable"
