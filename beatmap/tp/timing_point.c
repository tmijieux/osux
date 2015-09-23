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
#include "timing_point.h"

void tp_print(struct timing_point *tp)
{
    printf("%.15g,%.15g,%d,%d,%d,%d,%d,%d\r\n",
	   tp->offset,
	   tp->mpb,
	   tp->time_signature,
	   tp->sample_type,
	   tp->sample_set,
	   tp->volume,
	   tp->uninherited,
	   tp->kiai);
}

void tp_free(struct timing_point *tp)
{
    // nothing !
}
