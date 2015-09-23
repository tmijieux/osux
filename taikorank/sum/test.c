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

#include "sum.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
  struct heap * h = sum_new(5);
  sum_add(h, 15673);
  sum_add(h, 1654);
  sum_add(h, 1646828714);
  sum_add(h, 14);
  sum_add(h, -5);
  while(heap_size(h) > 0)
    {
      printf("%g\n", *(double *) heap_extract_max(h));
    }
}
