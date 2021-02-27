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
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;

// Producer consumer data structures
// Bounded buffer bigmatrix defined in prodcons.h
// Matrix ** bigmatrix;

Matrix *M1 = NULL;
Matrix *M2 = NULL;
Matrix *M3 = NULL;

volatile int ptr_to_fill = 0;
volatile int ptr_to_use = 0;
volatile int count = 0;

counter_t _produced_count;
counter_t _consumed_count;

// Bounded buffer put() get()
int put(Matrix *value)
{
  pthread_mutex_lock(&lock);
  if (value != NULL)
  {
    // double check if still not exceed the bounded buffer size
    if (count < BOUNDED_BUFFER_SIZE)
    {
      bigmatrix[ptr_to_fill] = value;
      //ptr_to_use = ptr_to_fill;                                  // TODO: look if we need it.
      ptr_to_fill = (ptr_to_fill + 1) % BOUNDED_BUFFER_SIZE; // move next and comes back to
      count++;                                               // may not need this becaptr_to_use counter
    }
  }
  pthread_mutex_unlock(&lock);
  return 0;
}

Matrix *get()
{
  Matrix *temp = NULL;
  pthread_mutex_lock(&lock);
  if (count > 0)
  {
    temp = bigmatrix[ptr_to_use];
    ptr_to_use = (ptr_to_use + 1) % BOUNDED_BUFFER_SIZE;
    count--;
  }
  pthread_mutex_unlock(&lock);

  return temp;
}

// Matrix PRODUCER worker thread
void *prod_worker(void *arg)
{
  ProdConsStats *produced_info = (ProdConsStats *)&arg; // changed the name

  while (get_cnt(&_produced_count) < NUMBER_OF_MATRICES)
  {
    pthread_mutex_lock(&mutex);

    // wait if it fulls
    while (count == BOUNDED_BUFFER_SIZE)
    {
      if (get_cnt(&_produced_count) == NUMBER_OF_MATRICES)
      { // check if >= or ==
        pthread_cond_signal(&full);
        pthread_mutex_unlock(&mutex);
        return 0;
      }
      pthread_cond_wait(&empty, &mutex);
    }

    // if (produced_info->matrixtotal == LOOPS)
    // {
    //   pthread_cond_broadcast(&full);
    // }
    //else
   // {

      // // TODO: double check if we need the matrix_mode
      // if (MATRIX_MODE == 0)
      // {
        M1 = GenMatrixRandom();
      // }
      // else if (MATRIX_MODE > 0)
      // {
      //   M1 = GenMatrixBySize(MATRIX_MODE, MATRIX_MODE); // where to get r and c?
      // }

      if (get_cnt(&_produced_count) < NUMBER_OF_MATRICES)
      {
        put(M1);
        produced_info->matrixtotal++;
        produced_info->sumtotal += SumMatrix(M1);
        increment_cnt(&_produced_count);
        pthread_cond_signal(&full);
      }
      pthread_mutex_unlock(&mutex);
  }
  pthread_cond_broadcast(&empty); // check if we need to wake up all the threads
  return 0;
}

// Matrix CONSUMER worker thread
void *cons_worker(void *arg)
{
  ProdConsStats *consumed_info = (ProdConsStats *)arg;
  int result = 0;
  while (get_cnt(&_consumed_count) < NUMBER_OF_MATRICES)
  {
    pthread_mutex_lock(&mutex);
    while (count == 0)
    {
      if (get_cnt(&_consumed_count) == NUMBER_OF_MATRICES)
      {
        pthread_cond_signal(&full);
        pthread_mutex_unlock(&mutex);
        return result;
      }
      pthread_cond_broadcast(&empty);
      pthread_cond_wait(&full, &mutex);
    }

    if (M1 == NULL && M2 == NULL)
    {
      //get value to M1
      M1 = get();
      consumed_info->matrixtotal++;
      consumed_info->sumtotal += SumMatrix(M1);
      increment_cnt(&_consumed_count);
    }
    else if (M1 != NULL && M2 == NULL)
    {
      //get value to M2
      M2 = get();
      consumed_info->matrixtotal++;
      consumed_info->sumtotal += SumMatrix(M2);
      increment_cnt(&_consumed_count);
    }
    else
    {
      //multiply M1 and M2
      M3 = MatrixMultiply(M1, M2);
      consumed_info->multtotal++;
      if (M3 != NULL)
      { //if M3 is not null free all the matrix
        DisplayMatrix(M1, stdout);
        printf("    X\n");
        DisplayMatrix(M2, stdout);
        printf("    =\n");
        DisplayMatrix(M3, stdout);
        printf("\n");
        FreeMatrix(M1);
        FreeMatrix(M2);
        FreeMatrix(M3);
        M1 = NULL;
        M2 = NULL;
        M3 = NULL;
      }
      else
      { // if M3 is null free M2 to get the next matrix
        FreeMatrix(M2);
        M2 = NULL;
      }
    }

    if (consumed_info->matrixtotal == BOUNDED_BUFFER_SIZE)
    { //if we run out of all the matrixes and M1 is not null, then free M1
      if (M1 != NULL)
      {
        FreeMatrix(M1);
        M1 = NULL;
      }
    }
    else
    {
      pthread_cond_signal(&empty);
    }
    pthread_mutex_unlock(&mutex);
  }
  return consumed_info;
}
