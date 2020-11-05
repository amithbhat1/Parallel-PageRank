/******************** Includes - Defines ****************/
//#include "pagerank_serial.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <string.h>
// #include "util.h"
//#include "mpi.h"
#include <stddef.h>
#include <mpi.h>

/******************** Defines ****************/
// Number of nodes
int N;
//Sum of sizes of To_id matixes
int len_con_mat=0;

typedef struct
{
  double p_t0;
  double *p_t1;
  double e;
  int con_size;
  int *To_id;
}Node;

// Convergence threashold and algorithm's parameter d  
double threshold, d;
double start, stop;

FILE *times;
// Table of node's data


//Matrix containing all connection data between all nodes. Its size is len_con_mat
int *con_mat;

double *global_pt0;
double *global_pt1;
double *global_big_p_t1;
double global_sum=0;

/***** Read graph connections from txt file *****/	

void Read_from_txt_file(char* filename,Node *Nodes)
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
			
				if (fscanf(fid,"%d\t%d\n", &from_idx,&to_idx))
				{
						Nodes[from_idx].con_size++;
						temp_size = Nodes[from_idx].con_size;
						Nodes[from_idx].To_id =(int*) realloc(Nodes[from_idx].To_id, temp_size * sizeof(int));
						Nodes[from_idx].To_id[temp_size - 1] = to_idx;
				}
		}
	
		// printf("End of connections insertion!\n");
		
		fclose(fid);
		
}


/***** Create P and E with equal probability *****/

void Random_P_E(Node *Nodes)
{
	
		int i;
		// Sum of P (it must be =1)
		double sum_P_1 = 0;
		// Sum of E (it must be =1)
		double sum_E_1 = 0; 
		
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
	
		// Exit if sum of Pt0 is !=1
		assert(sum_E_1 = 1);
		
}


/***** Main function *****/   

int main(int argc, char** argv)
{
		Node *Nodes;
		double *matrix_pt1;
		double *matrix_global;
		MPI_Init( &argc, &argv );
		int rank, size;
		MPI_Comm_rank( MPI_COMM_WORLD, &rank );
		MPI_Comm_size( MPI_COMM_WORLD, &size );
		//Create a new datatype, mpiNode, which is an the equivalent node struct for MPI. This is finally not used though.
		MPI_Datatype mpiNode;
		MPI_Type_create_resized( MPI_INT, 0, sizeof( Node ), &mpiNode );
		MPI_Type_commit( &mpiNode);
		

	
		// Check input arguments
		if (argc != 2)
		{
			printf("\nError in arguments! Argument required: graph filename\n");
			return 0;
		} 
		
			// get arguments 
			char filename[256];
			strcpy(filename, argv[1]);
			N = 916428;
		   	threshold = 0.0001;
		   	d = 0.85;
			
			int i, j, k;
			double totaltime;

			// a constant value contributed of all nodes with connectivity = 0
			// it's going to be addes to all node's new probability
			double sum = 0;
			double local_big_p_t1[N];
			
			// Allocate memory for N nodes
			Nodes = (Node*) malloc(N * sizeof(Node));
			//global_pt0=(double *)malloc(N*sizeof(double));
			matrix_pt1=(double *)malloc(N*sizeof(double));
			matrix_global=(double *)malloc(N*sizeof(double));
			global_big_p_t1=(double *)malloc(N*sizeof(double));
			

			for (i = 0; i < N; i++)
			{
				Nodes[i].con_size = 0;
				Nodes[i].To_id = (int*) malloc(sizeof(int));
				global_big_p_t1[i]=0.0;
				local_big_p_t1[i]=0.0;
				matrix_pt1[i]=0.0;
				matrix_global[i]=0.0;
				Nodes[i].p_t1=&(matrix_pt1[i]);
			}
		
			Read_from_txt_file(filename,Nodes);

         //Count the total number of the To_id matrices of all nodes. Later use this number to malloc a new array to put all this data in
			for (i = 0; i < N; i++)
			{
				len_con_mat+=Nodes[i].con_size;
			}
			//Construct a matrix which is solid in memory containing all the connection data. 
			con_mat=(int*)malloc(len_con_mat*sizeof(int));
         int con_index=0;
			for (i = 0; i < N; i++)
			{
				for (j=0;j<Nodes[i].con_size;j++)
				{
					con_mat[con_index]=Nodes[i].To_id[j];
					con_index++;
				}
				Nodes[i].To_id=&(con_mat[con_index-Nodes[i].con_size]);	
				
			}

			// set random probabilities
			Random_P_E(Nodes);
	

			int chunk_size = N/size;   //77/10 = 7.7
			int r = N % size;
	    		int start_index = rank*chunk_size;
			if(rank<r)
				start_index+=rank;
			else
				start_index+=r;
		
			int end_index = start_index + chunk_size;
        		if (rank < r) 
                		end_index++;

    		printf("For processor %d , start index is %d, end index is %d\n",rank,start_index,end_index);
		
			MPI_Barrier(MPI_COMM_WORLD);
			start = MPI_Wtime();
			
			
			/********** Start of algorithm **********/
			
			// Iterations counter
			int iterations = 0;
			int index;

			// Or any value > threshold
			double max_error = 5;
			double node_constant;   
			//printf("\nSerial version of Pagerank\n");
			
		
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
					//for (i = 0; i < N; i++)
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
										//local_big_p_t1[index]=*(Nodes[index].p_t1);
									}
								
							}
						
							else
							{
								// Contribute to all
								sum = sum + (double)Nodes[i].p_t0 / N;
							}
					}
					
					MPI_Reduce(matrix_pt1, matrix_global, N, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
					//MPI_Reduce(local_big_p_t1, matrix_pt1, N, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
					MPI_Reduce(&sum,&global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
				
				if ( rank == 0 ) {	
					max_error = -1;
					
					// Compute the new probabilities and find maximum error
					for (i = 0;i < N; i++)
					{
						*(Nodes[i].p_t1) = d * (*(Nodes[i].p_t1) + global_sum) + (1 - d) * Nodes[i].e;
						
							
							if (fabs(*(Nodes[i].p_t1) - Nodes[i].p_t0) > max_error)
							{
								max_error = fabs(*(Nodes[i].p_t1) - Nodes[i].p_t0);
							}
						
					}			
					
				printf("Max Error in iteration %d = %f\n", iterations+1, max_error);
				} //End of rank0
				
					iterations++;
					MPI_Bcast( &max_error, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
					MPI_Bcast( matrix_global, N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
			}
			stop = MPI_Wtime();

			FILE *f;
			f = fopen("xxxxoutput.txt", "w");

			// Print final probabilitities
			for (i = 0; i < N; i++)
			{
				fprintf(f,"Page Rank of Website %d  = %f\n",i,matrix_pt1[i]);
			}
			printf("\n");
			fclose(f);

		   
			
			  
			printf("\n");
			// FILE *f;
			// f = fopen("output.txt", "w");

			// // Print final probabilitities
			// for (i = 0; i < N; i++)
			// {
			// 	fprintf(f,"Page Rank of Website %d  = %f\n",i,Nodes[i].p_t0);
			// }
			// printf("\n");
			// fclose(f);
			     
			// Print final probabilitities
			MPI_Barrier(MPI_COMM_WORLD);
			
		if ( rank == 0 ) {	
			// Print no of iterations
			printf("Total iterations: %d\n", iterations);
			
			times = fopen("times.txt", "a");
			printf("Runtime = %f\n", stop-start);
			fprintf(times, "%f\n", (stop-start));
			printf("End of program!\n");
			MPI_Type_free(&mpiNode);
			//MPI_Abort(MPI_COMM_WORLD,0);
			
			
			
		}

	MPI_Finalize();
	
	//return (EXIT_SUCCESS);
}
