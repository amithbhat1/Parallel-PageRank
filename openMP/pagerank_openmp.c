#include "pagerank_serial.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <omp.h>



int N;               // Number of nodes 
double threshold, d; // Convergence threshold and algorithm's parameter d(damping factor)
Node *Nodes; // Table of node's data

double omp_start,omp_stop;

void Read_from_txt_file(char* filename)   //Read graph connections from txt file 
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

void Random_P_E() //Create P and E with equal probability
{
   int i;
   
   double sum_P_1 = 0; // Sum of P (it must be =1)
   double sum_E_1 = 0; // Sum of E (it must be =1)
       
   // Arrays initialization
   for (i = 0; i < N; i++)
   {
      Nodes[i].p_t0 = 0;
      Nodes[i].p_t1 = 1;
      Nodes[i].p_t1 = (double) Nodes[i].p_t1 / N;

      sum_P_1 = sum_P_1 + Nodes[i].p_t1;
        
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
   // Check input arguments
   if (argc != 2)
   {
      printf("Error in arguments! Please put the graph filename as argument!! \n");
      return 0;
   } 

   // get arguments 
   char filename[256];
   strcpy(filename, argv[1]);


   N = 916428; //Number of nodes
   threshold = 0.0001;
   d = 0.85;   // Damping factor
    
   int i, j, k;
   double totaltime;
   double node_constant;

   // a constant value contributed of all nodes with connectivity = 0
   // it's going to be added to all node's new probability
   double sum = 0;
    
   
   Nodes = (Node*) malloc(N * sizeof(Node));    // Allocating memory for N nodes
    
   #pragma omp parallel for private(i) shared(Nodes)
   for (i = 0; i < N; i++)
   {
      Nodes[i].con_size = 0;
      Nodes[i].To_id = (int*) malloc(sizeof(int));
   }

   Read_from_txt_file(filename);
    
   // set random probabilities
   Random_P_E();
    
   //gettimeofday(&start, NULL);

   // Measuring start time
   omp_start=omp_get_wtime();
    
	
   // ------------------  Core Algorithm Start ----------------- //
    
   // Iterations counter
   int iterations = 0;
   int index;
    
   // Or any value > threshold
   double max_error= 1;
   double error_log[100];
    
   // Continue if we don't have convergence yet
   while (max_error > threshold)
   {
      sum = 0;
      // Initialize P(t) and P(t + 1) values 
      #pragma omp parallel for private(i) shared(Nodes)
      for (i = 0; i < N; i++)
      {
         // Update the "old" P table with the new one 
         Nodes[i].p_t0 = Nodes[i].p_t1;  
         Nodes[i].p_t1 = 0;
      }

      // Find P for each webpage
      #pragma omp parallel for num_threads(16) private(i,j,index,node_constant) shared(Nodes)  schedule(guided) reduction(+:sum)
      for (i = 0; i < N; i++)
      {
         if (Nodes[i].con_size != 0)
         {
            node_constant=(double) Nodes[i].p_t0/Nodes[i].con_size;    
            // Compute the total probability, contributed by node's neighbors
            for (j = 0; j < Nodes[i].con_size; j++)
            {
               index = Nodes[i].To_id[j];
               #pragma omp atomic
	               Nodes[index].p_t1 = Nodes[index].p_t1 +node_constant;
            }

         }
            
        else
        {
         // Contribute to all
            sum += (double)Nodes[i].p_t0 / N;
        }
      }
        
      max_error = -1;
        
      // Compute the new probabilities and find maximum error
      #pragma omp parallel for private(i) shared(Nodes,max_error)
      for (i = 0;i < N; i++)
      {
         Nodes[i].p_t1 = d * (Nodes[i].p_t1 + sum) + (1 - d) * Nodes[i].e;
            
         if (fabs(Nodes[i].p_t1 - Nodes[i].p_t0) > max_error)
         {  
            max_error = fabs(Nodes[i].p_t1 - Nodes[i].p_t0);
         }
            
      }
      error_log[iterations]=max_error;  
      printf("Max Error in iteration %d = %f\n", iterations+1, max_error);
      iterations++;
   }
  
   // Measures end time
   omp_stop=omp_get_wtime();  

   FILE *f;
   f = fopen("output.txt", "w");

   // Print final probabilitities
   for (i = 0; i < N; i++)
   {
      fprintf(f,"Page Rank of Website %d  = %f\n",i,Nodes[i].p_t1);
   }
   printf("\n");
   fclose(f);

   // Print no of iterations
   printf("Total iterations: %d\n", iterations);
   printf("Total time: %f seconds\n", omp_stop-omp_start); 
   printf("End of program!\n"); 
   return (EXIT_SUCCESS);
}
