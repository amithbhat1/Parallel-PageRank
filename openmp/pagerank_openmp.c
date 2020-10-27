#include "Node.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <omp.h>
#include<sys/time.h>

// number of nodes
int N;

// Convergence threshold and algorithm's parameter d  
double threshold, d;

// Table of node's data
Node *Nodes;

double omp_start,omp_stop;

/***** Read graph connections from txt file *****/	
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

/***** Read P vector from txt file*****/	

void Read_P_from_txt_file()
{

   FILE *fid;
   double temp_P;
   int index = 0;

   fid = fopen("P.txt", "r");
   if (fid == NULL){printf("Error opening the Probabilities file\n");}

   while (!feof(fid))
   {
      // P's values are double!
      if (fscanf(fid,"%lf\n", &temp_P))
      {
         Nodes[index].p_t1 = temp_P;
         index++;   
      }
   }
   //printf("End of P insertion!");
   fclose(fid);	

}


/***** Read E vector from txt file*****/	

void Read_E_from_txt_file()
{

   FILE *fid;
   double temp_E;
   int index = 0;

   fid = fopen("E.txt", "r");
   if (fid == NULL)
      printf("Error opening the E file\n");

   while (!feof(fid))
   {
      // E's values are double!
      if (fscanf(fid,"%lf\n", &temp_E))
      {
         Nodes[index].e = temp_E;
         index++;   
      }
   }
   //printf("End of E insertion!");
   fclose(fid);	
}

/***** Create P and E with equal probability *****/

void Init_P_E()
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
      Nodes[i].p_t1 = 1;
      Nodes[i].p_t1 = (double) Nodes[i].p_t1 / N;

      sum_P_1 = sum_P_1 + Nodes[i].p_t1;
        
      Nodes[i].e = 1;
      Nodes[i].e = (double) Nodes[i].e / N;
      sum_E_1 = sum_E_1 + Nodes[i].e;
   }

   assert(sum_P_1 = 1);

   assert(sum_E_1 = 1);

}


/***** Main function *****/   

int main(int argc, char** argv)
{

   if (argc < 2)
   {
      printf("Error in arguments! Arguments required: graph filename\n");
      return 0;
   }

   char filename[256];
   strcpy(filename, argv[1]);
   N = 916428;
   threshold = 0.0001;
   d = 0.85; //generally taken as 0.85
    
   int i, j, k;
   double totaltime;
   double node_constant;

   // a constant value contributed of all nodes with connectivity = 0
   // it's going to be addes to all node's new probability
   double sum = 0;
    
   // Allocate memory for N nodes
   Nodes = (Node*) malloc(N * sizeof(Node));
    
   #pragma omp parallel for private(i) shared(Nodes)
   for (i = 0; i < N; i++)
   {
      Nodes[i].con_size = 0;
      Nodes[i].To_id = (int*) malloc(sizeof(int));
   }

   Read_from_txt_file(filename);
    
   // set  probabilities
   Init_P_E();
   // OR read probabilities from files
   //Read_P_from_txt_file();
   //Read_E_from_txt_file();
    
   omp_start=omp_get_wtime();
    
	
   /********** Start of algorithm **********/
    
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

      //distributing page rank to all pages connected to the given page
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
	               Nodes[index].p_t1 = Nodes[index].p_t1 + node_constant;
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
      //printf("Max Error in iteration %d = %f\n", iterations+1, max_error);
      iterations++;
   }

   omp_stop=omp_get_wtime();

   printf("\n");
   FILE *f;
   f = fopen("output.txt", "w");

   // Print final probabilitities
   for (i = 0; i < N; i++)
   {
      fprintf(f,"Page Rank of Website %d  = %f\n",i,Nodes[i].p_t1);
   }
   printf("\n");
   fclose(f);
   
   for(i=0;i<iterations;i++){
      printf("Max error in %d iteration is %f\n",i,error_log[i]);

   } 
   // Print no of iterations
   printf("Total iterations: %d\n", iterations);
   printf("Processing time: %f \n", omp_stop-omp_start); 
   printf("End of program!\n"); 
   return (EXIT_SUCCESS);
}
