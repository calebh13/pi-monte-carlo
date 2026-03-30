#!/bin/bash

for (( n = 1 ; n <= ((1 << 37)) ; n <<= 1 )); do
    ./main $n $1
done