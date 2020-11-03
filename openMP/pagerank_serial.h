//Struct for timestamps
struct timeval start,end;

//Struct used to define Nodes

typedef struct
{
  double p_t0; // Indicates the old table
  double p_t1;  // Indicates the new table
  double e;     // Constant
  int *To_id;   // Adjacency list (List of nodes this node is connected to)
  int con_size; // Number of nodes in the adjacency list
}Node;
