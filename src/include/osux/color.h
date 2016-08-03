#ifndef OSUX_COLOR_H
#define OSUX_COLOR_H
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

#include <stdio.h>
#include <stdint.h>

typedef struct osux_color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} osux_color;

void osux_color_print(FILE *f, osux_color *c, int id);

#endif // OSUX_COLOR_H
