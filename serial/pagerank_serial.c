#include "Node.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <assert.h>
#include <string.h>

// Dataset sourced from: https://snap.stanford.edu/data/web-Google.html

int N;

// Convergence threshold and algorithm's parameter d [ probability that website will be clicked in the first click by user(as opposed to clicking it from another website) ] 
double threshold, d;

// Table of node's data
Node *Nodes;

FILE *times;

//Read graph from txt file	

void Read_from_txt_file(char* filename)
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
   fclose(fid);

}

// Initialize Page Ranks with equal probability 

void Init_P_E()
{

   int i;
   // Sum of P (it must be =1)
   double sum_P_1 = 0;
   // Sum of E (it must be =1)
   double sum_E_1 = 0; 
       
   // Arrays initialization
   for (i = 0; i < N; i++)                   // section/task clause?
   {
      Nodes[i].p_t0 = 0;
      Nodes[i].p_t1 = 1;
      Nodes[i].p_t1 = (double) Nodes[i].p_t1 / N;

      sum_P_1 = sum_P_1 + Nodes[i].p_t1;
        
      Nodes[i].e = 1;
      Nodes[i].e = (double) Nodes[i].e / N;
      sum_E_1 = sum_E_1 + Nodes[i].e;
   }

   // Assert sum of probabilities/page rank is =1
    
   // Exit if sum of P is !=1
   assert(sum_P_1 = 1);
   assert(sum_E_1 = 1);

}  

int main(int argc, char** argv)
{
   // Check input arguments
   if (argc != 2)
   {
      printf("Error! argument required: graph filename\n");
      return 0;
   } 

   // get arguments 
   char filename[256];
   strcpy(filename, argv[1]);
   N = 916428; // number of nodes in dataset
   threshold = 0.0001;
   d = 0.85; // lambda value
    
   int i, j, k;
   double totaltime;

   // a constant value contributed of all nodes with connectivity = 0
   // it's going to be addes to all node's new probability
   double sum = 0;
    
   // Allocate memory for N nodes
   Nodes = (Node*) malloc(N * sizeof(Node));
    
   for (i = 0; i < N; i++)             // section/task clause?
   {
      Nodes[i].con_size = 0; //con_size: represents number of nodes a node is connected to 
      Nodes[i].To_id = (int*) malloc(sizeof(int));
   }

   Read_from_txt_file(filename);
    
   // initialization of probabilities of each node
   Init_P_E();
    
   gettimeofday(&start, NULL);

    
   // Iterations counter
   int iterations = 0;
   int index;
    
   // Or any value > threshold
   double max_error = 1;
   double node_constant;   // out-degree of node
  
 

   // Continue if we don't have convergence yet
   while (max_error > threshold)
   {
      sum = 0;
		
	   // Initialize P(t) and P(t + 1) values
      for (i = 0; i < N; i++)                               // parallel for 
      {
         // Update the "old" P table with the new one 
         Nodes[i].p_t0 = Nodes[i].p_t1;  
         Nodes[i].p_t1 = 0;
      }

      // Find P for each webpage
      for (i = 0; i < N; i++)                               // parallel for
      {
            
         if (Nodes[i].con_size != 0)
         {
                
            node_constant=Nodes[i].con_size;
            // Compute the total probability, contributed by node's neighbors
            for (j = 0; j < Nodes[i].con_size; j++)
            {
                index = Nodes[i].To_id[j];	
                Nodes[index].p_t1 = Nodes[index].p_t1 + (double) Nodes[i].p_t0 /node_constant ;
            }

         }
            
        else
        {
         // Contribute to all
            sum = sum + (double)Nodes[i].p_t0 / N;          // reduction clause?
        }
      }
        
      max_error = -1;
        
		// Compute the new probabilities and find maximum error
      for (i = 0;i < N; i++)
      {
         Nodes[i].p_t1 = d * (Nodes[i].p_t1 + sum) + (1 - d) * Nodes[i].e;
            
         if (fabs(Nodes[i].p_t1 - Nodes[i].p_t0) > max_error)
         {
            max_error = fabs(Nodes[i].p_t1 - Nodes[i].p_t0);
         }
            
      }
      printf("Max error in %d iteration is %f\n", iterations, max_error);
      iterations++;
   }

   gettimeofday(&end, NULL);
   
   FILE *f;
   f = fopen("output.txt", "w");

   // Print final probabilitities
   for (i = 0; i < N; i++)
   {
      fprintf(f,"Page Rank of Website %d  = %f\n",i,Nodes[i].p_t1);
   }
   fclose(f);

   // Print no of iterations
   printf("Total iterations: %d\n", iterations);
   times = fopen("times.txt", "a");

	
   totaltime = (((end.tv_usec - start.tv_usec) / 1.0e6 + end.tv_sec - start.tv_sec) * 1000) / 1000;
   printf("Processing time: %f secs\n", totaltime);
   fprintf(times, "%f\n", (totaltime));

   printf("End of program!\n");
    
   return (EXIT_SUCCESS);
}
