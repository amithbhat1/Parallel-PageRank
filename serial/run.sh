#!/bin/sh

counter=1

while [ $counter -le 100 ]
do
    ./a.out ${DATA_FILE}
    counter=$((counter+1))
done

