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

#include <stdio.h>
#include <stdlib.h>

#include "heap.h"

#define HEAP_BUFFER_SIZE 50

static int cmp_int(void *a, void *b)
{
	return (int)(long)a - (int)(long)b;
}

int main(int argc, char *argv[])
{
	struct heap *heap = heap_create(HEAP_BUFFER_SIZE, NULL,
					&cmp_int);

	heap_insert(heap, (void*)125);
	heap_insert(heap, (void*)5);
	heap_insert(heap, (void*)3);
	heap_insert(heap, (void*)7);
	heap_insert(heap, (void*)253);
	heap_insert(heap, (void*)2);
	heap_insert(heap, (void*)124);
	heap_insert(heap, (void*)7);

	while (heap_size(heap) > 0) {
		printf("%d\n", (int)(long) heap_extract_max(heap));
	}
	
	heap_destroy(heap);
	return 0;
}
