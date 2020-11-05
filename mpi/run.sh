#!/bin/sh

counter=1
PROCS=${1:-${NUM_PROCS}}
EXEC="mpi"

while [ $counter -le 100 ]
do
    mpiexec -n ${PROCS} ./${EXEC} ${DATA_FILE}
    counter=$((counter+1))
done

