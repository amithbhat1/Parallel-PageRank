First set environment variables 
    
        export NUM_PROCS=[number of processors]
        export DATA_FILE=[/path/to/file]

then perform the following
        make clean
        make 
        make run [NUM_PROCS=[N]] [DATA_FILE=[FILE]]

for running multiple times
        make run_mult