# Parallel-PageRank

Parallel implementation of PageRank algorithm using OpenMP and MPI.

Dataset sourced from : http://snap.stanford.edu/data/web-Google.html


Work Done :


Implemented a serial and 2 parallel versions of the PageRank algorithm, using OpenMP and MPI.


Outputs the number of iterations, time for execution and a file with the respective Page Ranks.


Compile & Execute instructions :


Serial : gcc pagerank_serial.c

         ./a.out ../web-Google.txt
         
OpenMP : gcc -fopenmp pagerank_openmp.c

         /a.out ../web-Google.txt
         
MPI   : mpic++ pagerank_mpi.c

        mpiexec -np [number_of_processors] /a.out ../web-Google.txt  [ Does not give the same output as Serial and OpenMP ]

        Running the MPI program with 1 processor will give the same output as the previous two programs.
        
