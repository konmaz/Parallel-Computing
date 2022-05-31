#!/bin/bash
threads=1
size=10
mode=1
repetitions=100

for i in $(seq 1 $repetitions)
do
./binaryTree $threads 2> >(sed -n "s/^.*Execution time:\s*\(\S*\).*$/\1/p") << EOF
$size $mode
EOF
done