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
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;


// Producer consumer data structures
// Bounded buffer bigmatrix defined in prodcons.h


// Matrix ** bigmatrix;

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
  _consumed_count++;
  return temp;
}

// Matrix PRODUCER worker thread
void *prod_worker(void *arg)
{
  ProdConsStats * produced_info = (ProdConsStats *) &arg;   // changed the name
  int num = 0;

  for (num = 0; num < LOOPS; num_prod++) {

    pthread_mutex_lock(&lock);

    while (_produced_count == MAX && finished == 0) {    // check with count or _produce_count   +  with LOOPS or MAX
      pthread_cond_wait(&empty, &lock);
    }

    if (produced_info -> matrixtotal == LOOPS) {
      pthread_cond_broadcast(&full);
    } else {
      if (MATRIX_MODE == 0) {
        M1 = GenMatrixRandom();
      } else if (MATRIX_MODE > 0) {
        M1 = GenMatrixBySize(MATRIX_MODE, MATRIX_MODE);   // where to get r and c? 
      }

      put(M1);
      produced_info -> matrixtotal++;
      produced_info -> sumtotal += SumMatrix(M1);
      pthread_con_signal(&full)                           // misleading with full vs fill?
    }
    pthread_mutex_unlock(&lock);
  }
  return produced_info;
}

// Matrix CONSUMER worker thread
void *cons_worker(void *arg)
{
  ProdConsStats* num_consumed = (ProdConsStats*) arg
  int i;
  for(i = 0; i< LOOPS; i++){
      pthread_mutex_lock(&lock);
      while (count == 0 && finished == 0)
        pthread_cond_wait(&full,&lock);
      if(M1 == NULL && M2 == NULL && finished != 1){
      //get value to M1
      M1 = get();
      num_consumed->sumtotal += SumMatrix(M1)
      num_consumed->matrixtotal++;
      }else if(M1 !=NULL&& M2 == NULL && finished != 1){
      //get value to M2
      M2 = get();
      num_consumed->sumtotal += SumMatrix(M1)
       num_consumed->matrixtotal++;
      }else{
      //multiply M1 and M2
      M3 = MatrixMultiply(M1,M2);
      num_consumed->multtotal++;
      if(M3 != NULL){//if M3 is not null free all the matrix
        DisplayMatrix(M1,stdout);
        printf("    X\n");
        DisplayMatrix(M2,stdout);
        printf("    =\n");
        DisplayMatrix(M3,stdout);
        printf("\n");
        free(M1);
        free(M2);
        free(M3);
        M1 = NULL;
        M2 = NULL;
        M3 = NULL;
      } else{// if M3 is null free M2 to get the next matrix
        free(M2);
        M2 = NULL;
      }
      }

      if( con_count->matrixtoal == LOOPS){//if we run out of all the matrixes and M1 is not null, then free M1
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
  return num_consumed;
}
