/*
 *  prodcons module
 *  Producer Consumer module
 *
 *  Implements routines for the producer consumer module based on
 *  chapter 30, section 2 of Operating Systems: Three Easy Pieces
 *
 *  Author: Xiuxiang Wu, Firn Tieanklin
 *  Date:   2/26/2021
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

Matrix *M1 = NULL;
Matrix *M2 = NULL;
Matrix *M3 = NULL;

volatile int ptr_to_fill = 0;
volatile int ptr_to_use = 0;
volatile int count = 0;

counter_t _produced_count;
counter_t _consumed_count;

// Bounded buffer put() get()

// add the matrix in the bounded buffer
int put(Matrix *value)
{
  pthread_mutex_lock(&lock);
  if (value != NULL)
  {
    // double check if still not exceed the bounded buffer size
    if (count < BOUNDED_BUFFER_SIZE)
    {
      // add to the next one in the bounded buffer
      bigmatrix[ptr_to_fill] = value;
      ptr_to_fill = (ptr_to_fill + 1) % BOUNDED_BUFFER_SIZE;
      count++;
    }
  }
  pthread_mutex_unlock(&lock);
  return 0;
}

Matrix *get()
{
  Matrix *temp = NULL;
  pthread_mutex_lock(&lock);
  //if the buffer is not empty, then get matrixes into the buffer.
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
  ProdConsStats *produced_info = (ProdConsStats *)arg;  // keep track with statistical info of producer

  while (get_cnt(&_produced_count) < NUMBER_OF_MATRICES)
  {
    pthread_mutex_lock(&mutex);

    // keep waiting if the bounded buffer still fulls
    while (count == BOUNDED_BUFFER_SIZE)
    {
      if (get_cnt(&_produced_count) == NUMBER_OF_MATRICES)
      {
        pthread_cond_signal(&full);   // send signal to the wating thread that buffer is full
        pthread_mutex_unlock(&mutex);
        return 0;
      }
      pthread_cond_wait(&empty, &mutex);  // release the lock wait for c.v. to be signaled
    }

    Matrix *mat = GenMatrixRandom();

    if (get_cnt(&_produced_count) < NUMBER_OF_MATRICES)
    {
      put(mat);

      // update statistical info of producer
      produced_info->matrixtotal++;
      produced_info->sumtotal += SumMatrix(mat);
      increment_cnt(&_produced_count);

      pthread_cond_signal(&full);    // sends signal that buffer's full
    }
    
    pthread_mutex_unlock(&mutex);
  }

  pthread_cond_broadcast(&empty);  // waking up all threads
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
    // check the buffer and get matrix to M1
    // if the buffer is empty, then wait
    while (count == 0)
    {
      if (get_cnt(&_consumed_count) >= NUMBER_OF_MATRICES)
      {
        pthread_cond_signal(&full);
        pthread_mutex_unlock(&mutex);
        return result;
      }
      pthread_cond_broadcast(&empty);
      pthread_cond_wait(&full, &mutex);
    }

    //get a matrix and assign to M1
    M1 = get();
    consumed_info->matrixtotal++;
    consumed_info->sumtotal += SumMatrix(M1);
    increment_cnt(&_consumed_count);

    while (count == 0)
    {
      if (get_cnt(&_consumed_count) >= NUMBER_OF_MATRICES || M1 == NULL)
      {
        pthread_cond_signal(&full);
        pthread_mutex_unlock(&mutex);
        return result;
      }
      pthread_cond_broadcast(&empty);
      pthread_cond_wait(&full, &mutex);
    }
    // check the buffer and get matrix to M2
    // if the buffer is empty, then wait
    while (count == 0)
    {
      if (get_cnt(&_consumed_count) >= NUMBER_OF_MATRICES || M2 == NULL)
      {
        pthread_cond_signal(&full);
        pthread_mutex_unlock(&mutex);
        return result;
      }
      pthread_cond_broadcast(&empty);
      pthread_cond_wait(&full, &mutex);
    }

    //get a matrix and assign to M2
    M2 = get();

    consumed_info->matrixtotal++;
    consumed_info->sumtotal += SumMatrix(M2);
    increment_cnt(&_consumed_count);

    //multiply M1 and M2
    M3 = MatrixMultiply(M1, M2);
    //Looking for matrixes that can multiply with M1
    while (M3 == NULL) 
    {
      // check the buffer and get matrix to M2
      // if the buffer is empty, then wait
      while (count == 0)
      {
        if (get_cnt(&_consumed_count) >= NUMBER_OF_MATRICES || M2 == NULL)
        {
          pthread_cond_signal(&full);
          pthread_mutex_unlock(&mutex);
          return result;
        }
        pthread_cond_broadcast(&empty);
        pthread_cond_wait(&full, &mutex);
      }
      if (M2 != NULL)
      {
        FreeMatrix(M2);
      }

      //get a matrix and assign to M2
      M2 = get();
      consumed_info->matrixtotal++;
      consumed_info->sumtotal += SumMatrix(M2);
      increment_cnt(&_consumed_count);
    }
    //Increment multtotal
    consumed_info->multtotal++;

    //if M3 is not null print out the multiplecation and free all the matrix
    DisplayMatrix(M1, stdout);
    printf("    X\n");
    DisplayMatrix(M2, stdout);
    printf("    =\n");
    DisplayMatrix(M3, stdout);
    printf("\n");
    FreeMatrix(M3);
    FreeMatrix(M2);
    FreeMatrix(M1);
    pthread_cond_signal(&empty);
    pthread_mutex_unlock(&mutex);
  }
  //wakeup sleeping thread. work is done.
  pthread_cond_broadcast(&full);
  return result;
}
