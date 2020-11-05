#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <stddef.h>
#include <mpi.h>



int N; 	// Number of nodes

typedef struct
{
  double p_t0;
  double *p_t1;
  double e;
  int con_size;
  int *To_id;
}Node;

 
double threshold, d; 	// Convergence threshold and algorithm's parameter d 
double start, stop;

double global_sum=0;

void Read_from_txt_file(char* filename,Node *Nodes)   //Read graph connections from txt file 
{ 
   FILE *fid;
   int from_idx, to_idx;
   int temp_size;
   
   fid = fopen(filename, "r");
   if (fid == NULL){
      printf("Error opening data file\n");
   }

   while (!feof(fid))
   {
      if (fscanf(fid,"%d\t%d\n", &from_idx,&to_idx)) // Filling the struct Nodes for each node in the graph
      {
         Nodes[from_idx].con_size++;
         temp_size = Nodes[from_idx].con_size;
         Nodes[from_idx].To_id =(int*) realloc(Nodes[from_idx].To_id, temp_size * sizeof(int));
         Nodes[from_idx].To_id[temp_size - 1] = to_idx;
      }
   }
   fclose(fid);
}


void Random_P_E(Node *Nodes) //Create P and E with equal probability
{
   int i;
   
   double sum_P_1 = 0; // Sum of P (it must be =1)
   double sum_E_1 = 0; // Sum of E (it must be =1)
       
   // Arrays initialization
   for (i = 0; i < N; i++)
   {
      Nodes[i].p_t0 = 0;
      *(Nodes[i].p_t1) = 1;
      *(Nodes[i].p_t1) = (double) *(Nodes[i].p_t1) / N;

      sum_P_1 = sum_P_1 + *(Nodes[i].p_t1);
        
      Nodes[i].e = 1;
      Nodes[i].e = (double) Nodes[i].e / N;
      sum_E_1 = sum_E_1 + Nodes[i].e;
   }

   // Assert sum of probabilities is =1
    
   // Exit if sum of P is !=1
   assert(sum_P_1 = 1);
      
   // Assert sum of E is =1
   // Exit if sum of Pt0 is !=1
   assert(sum_E_1 = 1);

}


int main(int argc, char** argv)
{
	Node *Nodes;
	double *matrix_pt1;
	double *matrix_global;

	// Initalize the MPI environment
	MPI_Init( &argc, &argv );
	int rank, size;
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	MPI_Comm_size( MPI_COMM_WORLD, &size );
	
	// Check input arguments
	if (argc != 2)
	{
		printf("Error in arguments! Please put the graph filename as argument!! \n");
		return 0;
	} 

	// get arguments 
	char filename[256];
	strcpy(filename, argv[1]);


	N = 916428; // Number of nodes
	threshold = 0.0001;
	d = 0.85; // Damping factor
	
	int i, j, k;
	double totaltime;

	// a constant value contributed of all nodes with connectivity = 0
	// it's going to be addes to all node's new probability
	double sum = 0;
	
	// Allocate memory for N nodes
	Nodes = (Node*) malloc(N * sizeof(Node));

	// Matrix to maintain P(t+1) values
	matrix_pt1=(double *)malloc(N*sizeof(double));	

	// Matrix to maintain the P(t+1) values calculated in the previous iteration by matrix_pt1
	matrix_global=(double *)malloc(N*sizeof(double));	


	for (i = 0; i < N; i++)
	{
		Nodes[i].con_size = 0;
		Nodes[i].To_id = (int*) malloc(sizeof(int));
		matrix_pt1[i]=0.0;
		matrix_global[i]=0.0;
		Nodes[i].p_t1=&(matrix_pt1[i]);
	}

	Read_from_txt_file(filename,Nodes);

	
	Random_P_E(Nodes); 	// set random probabilities

	// Determining the number of nodes for each processor

	int chunk_size = N/size;   // Eg : 77/10 = 7.7
	int r = N % size;
		int start_index = rank*chunk_size;
	if(rank<r)
		start_index+=rank;
	else
		start_index+=r;

	int end_index = start_index + chunk_size;
		if (rank < r) 
				end_index++;
	printf("\n");
	printf("For processor %d , start index is %d, end index is %d\n",rank,start_index,end_index);

	// Barrier to ensure all the processors start processing at same time [ For sake of measuring time]
	MPI_Barrier(MPI_COMM_WORLD);

	// Measures start time
	start = MPI_Wtime();

	// ------------------  Core Algorithm Start ----------------- //

	int iterations = 0; 	// counter
	int index;

	// Or any value > threshold
	double max_error = 1;
	double node_constant;   		

	// Continue if we don't have convergence yet
	while (max_error > threshold)
	{
		sum = 0;
			
		// Initialize P(t) and P(t + 1) values
		for (i = 0; i < N; i++)
		{
			// Update the "old" P table with the new one 
			Nodes[i].p_t0 = *(Nodes[i].p_t1);  
			*(Nodes[i].p_t1)= 0;	
		}
			
					
			// Find P for each webpage
			for (i = start_index; i < end_index; i++)
			{
				if (Nodes[i].con_size != 0)
				{
					node_constant=(double) Nodes[i].p_t0 /Nodes[i].con_size;
					
					// Compute the total probability, contributed by node's neighbors
					for (j = 0; j < Nodes[i].con_size; j++)
					{
						index = Nodes[i].To_id[j];	
						*(Nodes[index].p_t1) = *(Nodes[index].p_t1) + node_constant;
					}	
				}
			
				else
					sum = sum + (double)Nodes[i].p_t0 / N;	// Contribute to all
					
			}
			 
			// Transferring values from the local matrix_pt1 to matrix_global
			MPI_Reduce(matrix_pt1, matrix_global, N, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

			// Transferring values from the local sum variable to global sum variable
			MPI_Reduce(&sum,&global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
		
		if ( rank == 0 ) {	
			max_error = -1;
			
			// Compute the new probabilities and find maximum error
			for (i = 0;i < N; i++)
			{
				*(Nodes[i].p_t1) = d * (*(Nodes[i].p_t1) + global_sum) + (1 - d) * Nodes[i].e;	
				if (fabs(*(Nodes[i].p_t1) - Nodes[i].p_t0) > max_error)
					max_error = fabs(*(Nodes[i].p_t1) - Nodes[i].p_t0);				
			}
			
			printf("Max Error in iteration %d = %f\n", iterations+1, max_error);
		} 

		iterations++;

		// Broadcast the values of the matrix_global and max error variable
		MPI_Bcast( &max_error, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		MPI_Bcast( matrix_global, N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	}
	
	// Waiting to ensure all processes complete 
	MPI_Barrier(MPI_COMM_WORLD);

	// Measuring end time
	stop = MPI_Wtime();

	if (rank==0) {
		FILE *f;
		f = fopen("MPI_output.txt", "w");

		// Print final probabilitities
		for (i = 0; i < N; i++)
		{
			fprintf(f,"Page Rank of Website %d  = %f\n",i,*Nodes[i].p_t1);
		}
		printf("\n");
		fclose(f);
	
		printf("Total iterations: %d\n", iterations);
		

		printf("Total Time = %f seconds\n", stop-start);
		printf("End of program!\n");
	}

	MPI_Finalize();
}
