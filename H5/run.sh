#!/bin/bash

# Define the filename
filename='out.txt'
size=10
mode=1
repetitions=100

regex=":(.*)"
for i in {1..100}
do
./binaryTree "1" 2> >(sed -n "s/^.*Execution time:\s*\(\S*\).*$/\1/p") << EOF
$size $mode
EOF
done

# >> $filename
# printf "\n" >> $filename


# ./binaryTree "1" << EOF
# $size $random
# EOF
# | tee /dev/stderr > out.file
# ./binaryTree "1" 2> >(grep -i "Execution time:") << EOF
# $size $random
# EOF
#cat <(printf "10 1") - | ./binaryTree "1" 2> >(grep -i "Execution time:")


