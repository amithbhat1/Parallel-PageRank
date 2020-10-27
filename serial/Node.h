/***** Struct for timestamps *****/
struct timeval start,end;

/***** Struct used for Nodes data *****/

typedef struct
{
  double p_t0; // P(t) -> old page rank
  double p_t1;  // P(t+1) -> new page rank
  double e;   // 1/N value
  int *To_id; //pointer to adj list
  int con_size; // out-degree of the node
}Node;
