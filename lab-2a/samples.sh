#!/bin/sh

./map.sh input.txt
./map.sh -t 4 input.txt
./map.sh -c "cat -E" -t 3 -i input.txt
