#!/bin/bash

for (( i = 1 ; i <= (1 << 32) ; i <<= 1)); do
    ./main $i $1
done