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


#ifndef TIMING_POINT_H
#define TIMING_POINT_H

struct timing_point {
    double offset;

    union {
	double mpb; // millisecond per beats
	double svm; // slider velocity multiplier : percentage
    };

    int time_signature;
    int sample_type;
    int sample_set;
    int volume;
    
    int uninherited;
    int kiai;

    struct timing_point *last_uninherited;
};

void tp_print(struct timing_point *tp); // print one timing point
void tp_free(struct timing_point *tp); // free one timing point

#endif //TIMING_POINT_!H
