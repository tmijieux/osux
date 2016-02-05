/*
 * Copyright (c) 2015 Lucas Maugère, Thomas Mijieux
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COLOR_H
#define COLOR_H

#include <stdio.h>
#include <python2.7/Python.h>

struct color {
    union {
        unsigned int c;
	struct {
	    unsigned char r;
	    unsigned char g;
	    unsigned char b;
	    unsigned char a;
	};
    };
};

void col_print(FILE *f, struct color *c, int id);

#endif //COLOR_H
