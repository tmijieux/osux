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
#include <sorted_list.h>

static int compare_int(void *a_, void *b_)
{
	return (int)((unsigned long) a_ - (unsigned long) b_);
}

void print_slist(struct sorted_list *sl)
{
	int s = slist_size(sl);
	for (int i = 1; i <= s; i ++) {
		long a = (long) slist_get_data(sl, i);
		printf("%ld ", a);
	}
	puts("");
}

int main(int argc, char *argv[])
{
	struct sorted_list *sl = slist_create(0, &compare_int);

	slist_add_value(sl, (void*) 13L);
	print_slist(sl);
	
	slist_add_value(sl, (void*) 2L);
	print_slist(sl);
		
	slist_add_value(sl, (void*) 7L);
	print_slist(sl);
	
	slist_add_value(sl, (void*) 15L);
	print_slist(sl);
	
	slist_add_value(sl, (void*) 1L);
	print_slist(sl);

		
	slist_add_value(sl, (void*) 15L);
	print_slist(sl);
	
	slist_destroy(sl);
	return EXIT_SUCCESS;
}

