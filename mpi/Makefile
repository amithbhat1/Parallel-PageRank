CC=mpicc
EXECS=mpi

all: mpicc

mpicc: pagerank_mpi.c
	${CC} -o ${EXECS} pagerank_mpi.c 

run: ${EXECS}
	mpiexec -n ${NUM_PROCS} ${EXECS} ${DATA_FILE}

run_mult: run.sh
	./run.sh ${NUM_PROCS}

clean: 
	rm -f ${EXECS}
