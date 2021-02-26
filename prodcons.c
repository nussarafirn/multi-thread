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
int num_produced = 0;
int num_consumed = 0;
int num_multiplied = 0;


// Bounded buffer put() get()
int put(Matrix * value)
{
  
  bigmatrix[ptr_to_fill] = value;
  ptr_to_fill = (ptr_to_fill + 1) % MAX;  // move next and comes back to 
  num_produced++;                        // may not need this because counter
  
  return 0;
}

Matrix * get()
{
  return NULL;
}

// Matrix PRODUCER worker thread
void *prod_worker(void *arg)
{
  ProdConsStats * prod_count = (ProdConsStats *) &arg;

  int num_prod = 0;
  Matrix * mat;
  for (num_prod = 0; num_prod < LOOPS; num_prod++) {

    //lock

    if (// check if random ) {
      mat = GenMatrixRandom();
    } else if (//not random) {
      mat = GenMatrixBySize(r,c);   // where to get r and c? 
    }

    // unlock
  }
}

// Matrix CONSUMER worker thread
void *cons_worker(void *arg)
{
  return NULL;
}
