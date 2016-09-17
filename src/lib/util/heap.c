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
#include "osux/heap.h"

struct heap {
    int (*cmp)(void*, void*);
    unsigned heap_size;
    void **buf;
    int free_buf;
};

struct heap *heap_new(unsigned int buffer_size,	 void *buffer,
		      int (*cmp)(void*, void*))
{
    struct heap *heap = malloc(sizeof(*heap));

    heap->cmp = cmp;
    heap->heap_size = 0;

    if (buffer == NULL) {
	heap->buf = malloc(sizeof(*(heap->buf)) * (buffer_size + 1));
	heap->free_buf = 1;
    } else {
	heap->buf = buffer;
	heap->free_buf = 0;
    }
    return heap;
}

void heap_free(struct heap *heap)
{
    if (heap->free_buf)
	free(heap->buf);
    free(heap);
}

static int heap_greater_child(struct heap *heap, unsigned i)
{
    if (2*i == heap->heap_size)
	return 2*i;
    if (heap->cmp(heap->buf[2*i], heap->buf[2*i+1]) > 0)
	return 2*i;
    return 2*i+1;
}

static void heap_swap(struct heap *heap, int i, int k)
{
    void *tmp;
    tmp = heap->buf[i];
    heap->buf[i] = heap->buf[k];
    heap->buf[k] = tmp;
}

static int heap_extract_problem(struct heap *heap, unsigned i)
{
    return ((2*i == heap->heap_size &&
	     heap->cmp(heap->buf[2*i], heap->buf[i]) > 0) ||
	    (2*i+1 <= heap->heap_size &&
	     (heap->cmp(heap->buf[2*i], heap->buf[i]) > 0 ||
	      heap->cmp(heap->buf[2*i+1], heap->buf[i]) > 0)));
}

static int heap_extract_resolve(struct heap *heap, int i)
{
    int k = heap_greater_child(heap, i);
    heap_swap(heap, i, k);
    return k;
}

void *heap_extract_max(struct heap *heap)
{
    void *max = heap->buf[0];
    heap->buf[0] = heap->buf[--heap->heap_size];
    int i = 0;
    while (heap_extract_problem(heap, i)) {
	i = heap_extract_resolve(heap, i);
    }
    return max;
}

void *heap_max(struct heap *heap)
{
    return heap->buf[0];
}

static int heap_insert_problem(struct heap *heap, int i)
{
    return heap->cmp(heap->buf[i], heap->buf[i/2]) > 0;
}

static int heap_insert_resolve(struct heap *heap, int i)
{
    heap_swap(heap, i, i/2);
    return i/2;
}

void heap_insert(struct heap *heap, void *k)
{
    int i = heap->heap_size ++;
    heap->buf[i] = k;

    while (heap_insert_problem(heap, i)) {
	i = heap_insert_resolve(heap, i);
    }
}

size_t heap_size(struct heap *heap)
{
    return heap->heap_size;
}
