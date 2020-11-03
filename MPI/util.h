#ifndef _UTIL_H_
#define _UTIL_H_
#include <stdlib.h>
#include <stdio.h>

/*
 * contains utility code that initializes and checks vectors.
 */

/*
 * parses and verifies cli input.
 * @argc - argument count
 * @argv - arguments
 
* @return - vector length
 */
long parse_args(int argc,char **argv){

  if(argc < 2){
    return 0;
  }
  long vect_len = atoi(argv[1]); 
  return vect_len;
}//parse_args

/*
 * generates the vectors. assumes that vectors have been allocated.
 * @vect_len - the length of the vectors
 * @vect1 - vector initialized to length vect_len
 * @vect2 - vector initialized to length vect_len
 */
void init_vects(long vect_len,float *vect1,float *vect2){
  for(long i = 0; i < vect_len; i++){
    vect1[i] = i;
    vect2[i] = i+i; 
  }
}//init_vects

/*
 * @vect_len - the length of the vectors
 * @vect1 - vector initialized to length vect_len
 * @vect2 - vector initialized to length vect_len
 * @result - student generated addition of vect1 and vect2
 * @return - true if result is correct, otherwise false
 */
bool verify(long vect_len,float *vect1,float *vect2,float *result){
  for(long i = 0 ; i < vect_len; i++){
    float tmp_result = vect1[i] + vect2[i];
    if(tmp_result != result[i]){
      return false;
    }
  }
  return true;
}//verify
#endif
