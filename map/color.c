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
#include "color.h"

void col_print(struct color *c, int id)
{
    printf("Combo%d : %hhu,%hhu,%hhu\r\n", id,  c->r, c->g, c->b);
}

void map_parse_Colours(PyObject *d, struct map *m)
{
    if (!d)
	return;

    size_t size = PyList_Size(d);
    m->Colours = malloc(sizeof(*m->Colours) * size);
    m->colc = size;
    for (int i = 0; i < size; ++i) {
	PyObject *p = PyList_GetItem(PyList_GetItem(d, i), 1);
	
	m->Colours[i].r = PyInt_AsLong(PyList_GetItem(p, 0));
	m->Colours[i].g = PyInt_AsLong(PyList_GetItem(p, 1));
	m->Colours[i].b = PyInt_AsLong(PyList_GetItem(p, 2)) ;
    }
}
