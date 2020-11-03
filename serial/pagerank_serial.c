#include "pagerank_serial.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <string.h>


int N; // Number of nodes 
double threshold, d; // Convergence threshold and algorithm's parameter d(damping factor)
Node *Nodes; // Table of node's data


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

   // a constant value contributed of all nodes with connectivity = 0
   // it's going to be added to all node's new probability
   double sum = 0;
    
   
   Nodes = (Node*) malloc(N * sizeof(Node));    // Allocating memory for N nodes
    
   for (i = 0; i < N; i++)
   {
      Nodes[i].con_size = 0;
      Nodes[i].To_id = (int*) malloc(sizeof(int));
   }

   Read_from_txt_file(filename);
    
   // set random probabilities
   Random_P_E();

   // Measures start time    
   gettimeofday(&start, NULL);
    
	
   // ------------------  Core Algorithm Start ----------------- //
    
   int iterations = 0;  // counter
   int index;
    
   
   double max_error = 1;   // can be any value > threshold
   double node_constant;   
   

   // Continue if we don't have convergence yet
   while (max_error > threshold)
   {
      sum = 0;
		
	   // Initialize P(t) and P(t + 1) values
      for (i = 0; i < N; i++)
      {
         // Update the "old" P table with the new one 
         Nodes[i].p_t0 = Nodes[i].p_t1;  
         Nodes[i].p_t1 = 0;
      }

      // Find P for each webpage
      for (i = 0; i < N; i++)
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
            sum = sum + (double)Nodes[i].p_t0 / N; // Contribute to all
        
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
        
      printf("Max Error in iteration %d = %f\n", iterations+1, max_error);
      iterations++;
   }

   // Measuring end time
   gettimeofday(&end, NULL);

   FILE *f;
   f = fopen("output.txt", "w");

   // Print final probabilitities
   for (i = 0; i < N; i++)
   {
      fprintf(f,"Page Rank of Website %d  = %f\n",i,Nodes[i].p_t1);
   }
   printf("\n");
   fclose(f);

   printf("Total iterations: %d\n", iterations);
	
   totaltime = (((end.tv_usec - start.tv_usec) / 1.0e6 + end.tv_sec - start.tv_sec) * 1000) / 1000;

   printf("\nTotaltime = %f seconds\n", totaltime);
   printf("End of program!\n");
    
   return (EXIT_SUCCESS);
}
