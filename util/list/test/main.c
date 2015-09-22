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

#include "list.h"

int main(int argc, char *argv[])
{
    struct list *list = list_new(0);

    list_add(list, (void*) 4);
    list_add(list, (void*) 7);
    list_insert(list, 2, (void*) 3);

    for (int i = 1; i <= list_size(list); ++i)
	printf("%ld ", (long) list_get(list, i));
    puts("");
    list_free(list);
    return EXIT_SUCCESS;
}
