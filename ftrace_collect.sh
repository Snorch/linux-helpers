#!/bin/bash
if [ "$#" -ne 1 ]; then
	echo "usage: $0 <output/file/path>"
	exit 1
fi

echo "Ftrace collection started at: $(date)" > /sys/kernel/debug/tracing/trace_marker
cat /sys/kernel/debug/tracing/trace_pipe | tee $1
