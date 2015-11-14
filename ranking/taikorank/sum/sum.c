/*
 *  Copyright (©) 2015 Lucas Maugère, Thomas Mijieux
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <stdlib.h>

#include "sum.h"

static int inferior(void * x, void * y);
static int superior(void * x, void * y);
static double weight (double x, int i);

static struct heap * acc_sum_new (unsigned int size);
static void acc_sum_add (struct heap * heap, double x);
static double acc_sum_compute (struct heap * heap);

static struct heap * perf_sum_new (unsigned int size);
static void perf_sum_add (struct heap * heap, double x);
static double perf_sum_compute (struct heap * heap);

static struct heap * weight_sum_new (unsigned int size);
static void weight_sum_add (struct heap * heap, double x);
static double weight_sum_compute (struct heap * heap);

struct sum
{
  struct heap * h;
  void (* add) (struct heap *, double);
  double (* compute) (struct heap *);
};

//-----------------------------------------------

static int inferior(void * x, void * y)
{
  return (*(double *) x < *(double *) y);
}

//-----------------------------------------------

static int superior(void * x, void * y)
{
  return (*(double *) x > *(double *) y);
}

//-----------------------------------------------

static double weight (double x, int i)
{
  return x * (100 - (i * 100. / MAX_NB_WEIGHTED)) / 100;
}

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------

/*
  sum version for ACCURACY !
  more accurate, but slower
 */

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------

static struct heap * acc_sum_new (unsigned int size)
{
  return heap_new(size, NULL, inferior);
}

//-----------------------------------------------

static void acc_sum_add (struct heap * heap, double x)
{
  double * data = malloc(sizeof(double));
  *data = x;
  heap_insert(heap, data);
}

//-----------------------------------------------

static double acc_sum_compute (struct heap * heap)
{
  if (heap_size(heap) == 0)
    {
      heap_free(heap);
      return 0;
    }
  
  while (heap_size(heap) > 1)
    {
      double * min1 = heap_extract_max(heap);
      double * min2 = heap_extract_max(heap);
      *min1 += *min2;
      heap_insert(heap, min1);
      free(min2);
    }
  double * sum = heap_extract_max(heap);
  double true_sum = *sum;
  free(sum);
  heap_free(heap);
  return true_sum;
}

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------

/*
  sum version for PERFORMANCE !
  less accurate, but faster
 */

//-----------------------------------------------

static struct heap * perf_sum_new (unsigned int size)
{
  double * sum = malloc(sizeof(double));
  *sum = 0;
  return (struct heap *) sum;
}

//-----------------------------------------------

static void perf_sum_add (struct heap * heap, double x)
{
  double * sum = (double *) heap;
  *sum += x;
}

//-----------------------------------------------

static double perf_sum_compute (struct heap * heap)
{
  double * sum = (double *) heap;
  double true_sum = *sum;
  free(sum);
  return true_sum;
}

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------

/*
  weighted sum version
  bigger elements are weighted more
 */

//-----------------------------------------------

static struct heap * weight_sum_new (unsigned int size)
{
  return heap_new(size, NULL, superior);
}

//-----------------------------------------------

static void weight_sum_add (struct heap * heap, double x)
{
  double * data = malloc(sizeof(double));
  *data = x;
  heap_insert(heap, data);
}

//-----------------------------------------------

static double weight_sum_compute (struct heap * heap)
{
  if (heap_size(heap) == 0)
    {
      heap_free(heap);
      return 0;
    }

  int nb_weighted = 0;
  struct sum * sum = sum_new(MAX_NB_WEIGHTED, PERF);

  while (heap_size(heap) > 0)
    {
      double * max = heap_extract_max(heap);
      
      if (nb_weighted < MAX_NB_WEIGHTED)
	{
	  sum_add(sum, weight(*max, nb_weighted));
	  nb_weighted++;
	}

      free(max);
    }

  heap_free(heap);
  return sum_compute(sum);
}

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------

/*
  sum module
 */

//-----------------------------------------------

struct sum * sum_new (unsigned int size, enum sum_type type)
{
  struct sum * sum = malloc(sizeof(struct sum));

  if (type == DEFAULT)
    type = PERF;
  
  switch (type)
    {
    case PERF:
      sum->h       = perf_sum_new(size);
      sum->add     = perf_sum_add;
      sum->compute = perf_sum_compute;
      break;
    case ACC:
      sum->h       = acc_sum_new(size);
      sum->add     = acc_sum_add;
      sum->compute = acc_sum_compute;
      break;
    case WEIGHT:
      sum->h       = weight_sum_new(size);
      sum->add     = weight_sum_add;
      sum->compute = weight_sum_compute;
      break;
    default:
      free(sum);
      sum = NULL;
      break;
    }
  return sum;
}

//-----------------------------------------------

void sum_add (struct sum * s, double x)
{
  s->add(s->h, x);
}

//-----------------------------------------------

double sum_compute (struct sum * s)
{
  double true_sum = s->compute(s->h);
  free(s);
  return true_sum;
}
