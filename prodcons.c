/*
 *  prodcons module
 *  Producer Consumer module
 *
 *  Implements routines for the producer consumer module based on
 *  chapter 30, section 2 of Operating Systems: Three Easy Pieces
 *
 *  University of Washington, Tacoma
 *  TCSS 422 - Operating Systems
 */

// Include only libraries for this module
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "counter.h"
#include "matrix.h"
#include "pcmatrix.h"
#include "prodcons.h"

// Define Locks and Condition variables here
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empt = PTHREAD_COND_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;


// Producer consumer data structures
// Bounded buffer bigmatrix defined in prodcons.h


// Matrix ** bigmatrix;

Matrix** bigmatrix[MAX];

Matrix* M1 = NULL;
Matrix* M2 = NULL;
Matrix* M3 = NULL;



int ptr_to_fill = 0;
int _produced_count = 0;
int _consumed_count = 0;
// int num_multiplied = 0;
int fill = 0;
int use = 0;
int count = 0;
int finished = 0;

// Bounded buffer put() get()
int put(Matrix * value)
{
  
  bigmatrix[ptr_to_fill] = value;
  ptr_to_fill = (ptr_to_fill + 1) % MAX;  // move next and comes back to 
  _produced_count++;   
  count++;                     // may not need this because counter
  
  return 0;
}

Matrix * get()
{
  int temp = bigmatrix[use];
  use = (use +1) % MAX;
  count--;
  _consumed_count = 0;
  return temp;
}

// Matrix PRODUCER worker thread
void *prod_worker(void *arg)
{
  ProdConsStats * num_produced = (ProdConsStats *) &arg;
  int num_prod = 0;
  Matrix * mat;

  for (num_prod = 0; num_prod < LOOPS; num_prod++) {

    pthread_mutex_lock(&lock);

    if (MATRIX_MODE == 0) {
      mat = GenMatrixRandom();
    } else if (MATRIX_MODE > 0) {
      mat = GenMatrixBySize(MATRIX_MODE, MATRIX_MODE);   // where to get r and c? 
    }
    
    
    
    
    pthread_mutex_unlock(&lock);
  }
  return num_produced;
}

// Matrix CONSUMER worker thread
void *cons_worker(void *arg)
{
  int i;
  for(i = 0; i< LOOPS; i++){
      pthread_mutex_lock(&lock);
      while (count == 0 && finished == 0)
        pthread_cond_wait(&full,&lock);
      int **bm = *&bigmatrix;
      if(M1 == NULL && M2 == NULL && finished != 1){
      //get value to M1
      M1 = get();
    
      }else if(M1 !=NULL&& M2 == NULL && finished != 1){
      //get value to M2
      M2 = get();
    
      }else{
      //multiply M1 and M2
      M3 = MatrixMultiply(M1,M2);
      }
      if( con_count->matrixtoal == LOOPS){
        if(M1 != NULL){
          FreeMatrix(M1);
          M1 = NULL;
        }
        finished = 1;
      } else {
        pthread_con_signal(&empty);
      }
      pthread_mutex_unlock(&lock);

      printf("%d\n", temp);
      FreeMatrix(M1, M2, M3);
  }
  return NULL;
}
