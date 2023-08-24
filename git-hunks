#!/usr/bin/python3
import sys, re

re_file = r'^diff --git a/(\S\S*)\s.*$'
re_line = r'^@@ -(\d\d*)\D.*@@.*$'

if __name__ == '__main__':
    if len(sys.argv) != 1 and len(sys.argv) != 2:
        print(f'usage: {sys.argv[0]} <path/to/file>')
        print(f'usage: <command> | {sys.argv[0]}')
        exit(1)

    with open(sys.stdin.fileno() if len(sys.argv) == 1 else sys.argv[1], 'r') as fi:
        file_name = None
        for line in fi:
            file_matches = re.findall(re_file, line)
            if len(file_matches) == 1:
                file_name = file_matches[0]
                continue

            if file_name == None:
                continue

            line_matches = re.findall(re_line, line)
            if len(line_matches) == 1:
                print(f'{file_name} {line_matches[0]}')
